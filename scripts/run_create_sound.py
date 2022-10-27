#!/usr/bin/env python
#
# script to generate sound from a SPECFEM3D ascii output trace file
#
from __future__ import print_function

import os
import sys

import numpy as np
import matplotlib.pyplot as plt
import scipy

import obspy
from obspy.signal.filter import envelope

from scipy.io import wavfile

####################################################################
# USER SETTINGS

# hearing is most sensitive in the 2000 - 5000 Hz frequency range, the min/max would be 20 - 20,000 Hz
audio_lowest_freq   = 2     # Hz
audio_highest_freq  = 6000  # Hz

# audio trace with higher sampling rate
audio_sampling_rate = 32000 # Hz

# augments traces with generated signals
add_augmented = True
augmented_percentage = 0.002
# number of evaluating bandpass filters
nbandpass_freq = 50 # 100
# generates signals on a harmonic scale for sonic augmentation
harmonizer = False
harmonizer_scale_type = 3 # 0==C-E-G,1==C-D#-G,2==C-D#-G-A,3==D-F/D-A,else==C-G/C-D#

# adds white noise
add_noise = False
noise_amplification = 10.0

# adds reverb
add_reverb = True
reverb_dry_wet_percentage = 0.2 #0.6  # percentage between input and reverbed signal
reverb_delay = 0.2 # 0.4 for longer delay
reverb_decay = 0.5 # 0.95 for longer reverb

# adds a shift in position
add_localization = False

####################################################################

def pitch(freq):
    # determines pitch and harmonics
    global harmonizer_scale_type
    A4 = 440.0
    C0 = A4 * pow(2, -4.75)
    name = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]

    h = round(12 * np.log2(freq/C0))
    octave = int(h / 12)
    halfstep_n = int(h % 12)

    # A4: f_A4 = 440 Hz  -> A5 = 2 * f_A4 = 880 -> A6 = 4 * f_A4 = 1760 -> ..
    # harmonic scale
    # from: https://pages.mtu.edu/~suits/scales.html
    harmonics = [ 1.0, 25.0/24.0, 9.0/8.0, 6.0/5.0, 5.0/4.0, 4.0/3.0, 45.0/32.0, 3.0/2.0, 8.0/5.0, 5.0/3.0, 9.0/5.0, 15.0/8.0, 2.0]

    #h = round(12 * np.log2(freq_scaled / A4) )
    #octave = int(h / 12)
    #halfstep_n = int(h % 12)

    # limits harmonics
    if harmonizer_scale_type == 0:
        # limits to: C -> E -> G (major)
        if halfstep_n <= 3: halfstep_n = 0
        elif halfstep_n <= 6: halfstep_n = 4
        else: halfstep_n = 7

    elif harmonizer_scale_type == 1:
        # limits to: C -> D# -> G (minor)
        if halfstep_n <= 2: halfstep_n = 0
        elif halfstep_n <= 6: halfstep_n = 3
        else: halfstep_n = 7

    elif harmonizer_scale_type == 2:
        # limits to: C -> D# -> G -> A#
        if halfstep_n <= 2: halfstep_n = 0
        elif halfstep_n <= 6: halfstep_n = 3
        elif halfstep_n <= 9: halfstep_n = 7
        else: halfstep_n = 10

    elif harmonizer_scale_type == 3:
        if octave <= 3:
            # limits to: D -> F
            if halfstep_n <= 4: halfstep_n = 2
            else: halfstep_n = 5
        else:
            # limits to: D -> A
            if halfstep_n <= 8: halfstep_n = 2
            else: halfstep_n = 9

    else:
        if octave <= 3:
            # limits to: C -> G
            if halfstep_n <= 6: halfstep_n = 0
            else: halfstep_n = 7
        else:
            # limits to: C -> D#
            if halfstep_n <= 2: halfstep_n = 0
            else: halfstep_n = 3

    #print("pitch: ",pitch(freq_scaled), h,octave,halfstep_n,"just scale = ",A4*pow(2,h))
    harmonic = harmonics[halfstep_n] * (C0 * pow(2,octave))
    pitch_name = name[halfstep_n] + str(octave)

    return harmonic,pitch_name


def audible_frequency(f,fmin,fmax,audio_lowest_freq,audio_highest_freq,speed_up_factor):
    # gets frequency in audible range
    global harmonizer

    # frequency scaling into hearing range
    #  fmin -> audio_lowest
    #  fmax -> audio_highest
    frange = fmax - fmin
    audio_range = audio_highest_freq - audio_lowest_freq

    # scales  [fmin,fmax] -> [audio_lowest,audio_highest]
    freq_scaled = (f - fmin) / frange * audio_range + audio_lowest_freq

    if harmonizer:
        # gets harmonic tune
        fharmonic,name = pitch(freq_scaled)
        print("  harmonic = ",fharmonic,name)
        # sets to harmonic
        freq_scaled = fharmonic

    # when shrinking the audio length from the true seismogram length, the frequencies will get scaled up
    # we compensate for that factor
    freq_scaled = freq_scaled / speed_up_factor

    return freq_scaled


def generate_signal(freq_audio,time_audio):
    # sine wave
    signal_sine = np.sin(2.0 * np.pi * freq_audio * time_audio)

    if harmonizer:
        # adds harmonics
        signal1 = 1.2 * np.sin(1.0 * np.pi * freq_audio * time_audio)     # C 1 octave below
        signal2 = 0.4 * np.sin(6.0/5.0 * np.pi * freq_audio * time_audio) # D#
        signal3 = 0.4 * np.sin(3.0/2.0 * np.pi * freq_audio * time_audio) # G
        signal4 = 0.0 # 0.2 * np.sin(5.0/3.0 * np.pi * freq_audio * time_audio) # A
        signal_sine += signal1 + signal2 + signal3 + signal4

    # saw
    signal_saw = 0.2 * scipy.signal.sawtooth(2.0 * np.pi * freq_audio * time_audio)

    # triangle
    signal_tri = 0.2 * np.abs(scipy.signal.sawtooth(2.0 * np.pi * freq_audio * time_audio))

    # sound signal
    signal = signal_sine + signal_saw + signal_tri

    return signal


def plot_spectrum(trace):
    """
    plot power spectrum
    """
    # trace data
    signal = trace.data.copy()

    # sampling rate
    fs = 1.0/trace.stats.delta
    print("sampling rate: ",fs)

    # Nyquist frequency
    freq = 0.5 * fs
    print("Nyquist frequency: ",freq)

    # frequency x-axis range
    T1 = 500.0
    T2 = 10.0
    f1 = 1./(T1)
    f2 = 1./(T2)

    # power spectral density
    f, freq_density = scipy.signal.periodogram(signal,fs)

    # power spectrum
    f, freq_spec = scipy.signal.periodogram(signal, fs, 'flattop', scaling='spectrum')
    #linear_spec = np.sqrt(freq_spec)

    # plotting
    fig = plt.figure(figsize=(14,8))

    a1 = fig.add_subplot(211)
    plt.semilogy(f, freq_density)
    plt.ylabel('Power spectral density')
    plt.xlabel('frequency [Hz]')
    plt.xlim([f1,f2])

    a2 = fig.add_subplot(212)
    plt.semilogy(f, freq_spec)
    plt.ylabel('Power spectrum')

    #plt.semilogy(f, linear_spec)
    #plt.ylabel('Linear spectrum')
    plt.xlabel('frequency [Hz]')
    plt.xlim([f1,f2])
    plt.show()

    signal = None


def find_nearest(array, value):
    array = np.asarray(array)
    idx = (np.abs(array - value)).argmin()
    return idx,array[idx]


def shift_array(array, num, fill_value=0.0):
    array = np.roll(array,num)
    if num < 0:
        array[num:] = fill_value
    elif num > 0:
        array[:num] = fill_value
    return array


def get_amplitude(data,deltat,freq):
    # Frequency domain
    FT = np.fft.rfft(data)
    # amplitude spectrum
    FT = np.abs(FT)
    # Frequency axis for plotting
    FT_freqs = np.fft.rfftfreq(len(data),d=deltat)

    # gets index of maximum
    #result = np.where(FT == np.amax(FT))
    #idx = result[0]
    #fmax = FT_freqs[idx]

    # gets index of freq
    idx,f = find_nearest(FT_freqs,freq)
    amp = FT[idx]
    return amp,idx,f


## see original: https://github.com/Rishikeshdaoo/Reverberator/blob/master/Reverberator/src/com/rishi/reverb/Reverberation.java

def comb_filter(buffer, delay, decay):
    # Comb Filter
    filtered = buffer.copy()
    for i in range(len(buffer) - delay):
        filtered[i+delay] += filtered[i] * decay
    return filtered


def allpass_filter(buffer,sampling_rate=8000):
    # All Pass Filter
    decay  = 0.131
    delay = int (0.089 * sampling_rate)
    filtered = np.zeros(len(buffer))
    for i in range(len(buffer)):
        filtered[i] = buffer[i]
        if i - delay >= 0: filtered[i] += - decay * filtered[i-delay]
        if i - delay >= 1 and i+20-delay < len(buffer): filtered[i] += decay * filtered[i+20-delay]
    # normalizes filtered signal
    filtered = filtered / np.max(np.abs(filtered))
    return filtered


def reverb_filter(buffer, sampling_rate=8000):
    global reverb_delay,reverb_decay,reverb_dry_wet_percentage
    # Schroeder reverberator
    # delays
    delay1 = int(reverb_delay * sampling_rate) # number of samples for 0.1 s
    delay2 = int((delay1 - 0.011) * sampling_rate)
    delay3 = int((delay1 + 0.019) * sampling_rate)
    delay4 = int((delay1 - 0.007) * sampling_rate)
    # decay factors
    decay1 = reverb_decay
    decay2 = decay1 - 0.13
    decay3 = decay1 - 0.27
    decay4 = decay1 - 0.31
    # dry/wet
    print("reverb:")
    print("  delay 1 = ",delay1)
    print("  decay 1 = ",decay1)
    print("  dry/wet percentage = ",reverb_dry_wet_percentage)
    print("")

    # Schroeder reverberator
    out1 = comb_filter(buffer,delay1,decay1)
    out2 = comb_filter(buffer,delay2,decay2)
    out3 = comb_filter(buffer,delay3,decay3)
    out4 = comb_filter(buffer,delay4,decay4)
    # adds comb filters
    comb_out = out1 + out2 + out3 + out4

    # dry/wet mixure
    perc = reverb_dry_wet_percentage
    mix = (1.0 - perc) * buffer + perc * comb_out
    # all pass
    out1 = allpass_filter(mix,sampling_rate)
    out2 = allpass_filter(out1,sampling_rate)

    # Schroeder
    filtered = out2
    # mix
    #filtered = (1.3 * buffer) + out2
    return filtered


def noise_filter(array):
    # adds white noise to array
    global noise_amplification
    print("noise filter:")
    print("  noise amplification factor = ",noise_amplification)
    print("")

    # adds noise
    npts = len(array)
    n = np.exp(1j*np.random.uniform(0, 2*np.pi, (npts,)))
    dat_noise = np.fft.ifft(n).real
    noise_added = array + noise_amplification * dat_noise
    return noise_added


## see original: http://zulko.github.io/blog/2014/03/29/soundstretching-and-pitch-shifting-in-python/

def stretch_array(array, factor, window_size, h):
    # stretches array by a factor

    # makes sure window_size is even
    if window_size % 2 == 1: window_size += 1
    window_size_half = int(window_size / 2)

    phase  = np.zeros(window_size)
    window = np.hanning(window_size)
    stretched = np.zeros( int(len(array)/factor) + window_size)

    # input array with padding at beginning/end to center windows around index i
    array_padded = np.zeros(len(array) + window_size)
    array_padded[window_size_half:window_size_half + len(array)] = array

    for i in np.arange(0, len(array_padded) - (window_size+h), int(h*factor)):

        # two potentially overlapping subarrays
        sub1 = array_padded[i:(i + window_size)]          # array[i:(i + window_size)]
        sub2 = array_padded[i + h:(i + window_size + h)] # array[i + h:(i + window_size + h)]

        # resynchronize the second array on the first
        FT1 =  np.fft.fft(window * sub1)
        FT2 =  np.fft.fft(window * sub2)
        FT1[FT1 == 0.0] = 1.0  # sets zero elements to 1.0
        phase = (phase + np.angle(FT2/FT1)) % 2.0 * np.pi
        rephased = np.fft.ifft(np.abs(FT2) * np.exp(1j*phase))

        # add to result
        idx = int(i/factor)
        stretched[idx : idx + window_size] += window * rephased.real

    # normalizes to 16bit
    #stretched = ((2**(16-4)) * result/result.max())
    #stretched = result.astype('int16')

    return stretched

def speedup_array(array, factor):
    # multiplies sound speed by some factor
    indices = np.round( np.arange(0, len(array), factor) )
    indices = indices[indices < len(array)].astype(int)
    return array[ indices.astype(int) ]


def shift_pitch(array, n, window_size, h):
    # Changes the pitch by n semitones
    factor = 2**(1.0 * n / 12.0)
    stretched = stretch_array(array, 1.0/factor, window_size, h)
    shifted = speedup_array(stretched[window_size:], factor)
    return shifted


def multiply_with_amplitude_spectrum(signal,tr_filt,freq_c,FT_all,FT_all_freqs,speed_up_factor):
    global audio_sampling_rate
    # overall amplitude
    idx,f = find_nearest(FT_all_freqs,freq_c)
    amp_all = FT_all[idx]
    amp_max = np.max(np.abs(FT_all))
    print("  amplitude = ",amp_all," freq nearest = ",f,idx," max amp = ",amp_max)

    #amp = pow(amp_all,2.0)
    #signal = amp * signal

    # original length
    npts = tr_filt.stats.npts
    deltat = tr_filt.stats.delta
    # window size for center frequency
    wlen = int(1.0/freq_c * tr_filt.stats.sampling_rate)
    if wlen % 2 == 1: wlen += 1
    if wlen < 100: wlen = 100
    wlen_half = int(wlen / 2)
    print("  amplitude moving window: length = ",wlen," total trace length = ",npts)

    amp = np.zeros(npts)
    for i in range(npts):
        window = np.zeros(wlen)
        amp_w = 1.0
        if i >= wlen_half and i < npts - wlen_half:
            # window centered around i
            window = tr_filt.data[i-wlen_half:i+wlen_half-1]
            (amp_w,idx,f) = get_amplitude(window,deltat,freq_c)

            #print("  get amplitude = ",amp,freq," freq nearest = ",f,idx)

            # normalizes
            amp_w = amp_w / amp_all
            if amp_w > 1.0: amp_w = 1.0

        # stores in trace
        amp[i] = amp_w

    # plots
    if 1 == 0:
        t = np.arange(0, tr_filt.stats.npts/tr_filt.stats.sampling_rate, 1.0/tr_filt.stats.sampling_rate)
        plt.plot(t, tr_filt.data / np.max(np.abs(tr_filt.data)), 'k')
        plt.plot(t, amp, 'b')
        plt.ylabel('Amplitude spectrum %f' % freq_c)
        plt.xlabel('Time [s]')
        plt.show()

    # interpolate to audio sampling
    tr_tmp_amp = tr_filt.copy()
    tr_tmp_amp.data = amp
    tr_tmp_amp.interpolate(sampling_rate=audio_sampling_rate/speed_up_factor)

    # modify signal strength
    signal = tr_tmp_amp.data * signal

    # free
    tr_tmp_amp = None

    return signal


def create_audio_pitched(tr,speed_up_factor,fmin,fmax,station_name):
    """
    shifts pitch of raw input to be in audible range
    """
    global audio_sampling_rate

    print("creating pitched audio:")
    print("  using interpolation to audio sampling range")
    print("")

    # raw input to audio
    tr_raw = tr.copy()
    tr_raw.interpolate(sampling_rate=audio_sampling_rate/speed_up_factor)

    print("  raw signal:")
    print("  original duration : ",tr.stats.npts * tr.stats.delta)
    print("  new audio duration: ",tr_raw.stats.npts * tr_raw.stats.delta / speed_up_factor)
    print("")
    print("  speedup factor = ",speed_up_factor)
    print("")
    print("  raw range fmin = ",fmin * speed_up_factor)
    print("  raw range fmax = ",fmax * speed_up_factor)
    print("")

    # saves raw input
    tr_raw = tr.copy()
    tr_raw.interpolate(sampling_rate=audio_sampling_rate/speed_up_factor)

    # file output
    filename = station_name + "sonic.raw.wav"
    tr_raw.write(filename, format='WAV', width=4, rescale=True, framerate=audio_sampling_rate)
    print("  audio file: ",filename)
    print("")

    # shifts pitch
    print("  shifting pitch:")
    tr_pitched = tr.copy()
    tr_pitched.interpolate(sampling_rate=audio_sampling_rate/speed_up_factor)
    # taper
    tr_pitched.taper(max_percentage=0.05)

    # calulcates number of halftones to move from current max to audio max
    fmax_raw = fmax * speed_up_factor
    fmax_audible = 20000.0
    nhalftones = int(round(12 * np.log2(fmax_audible/fmax_raw)))

    # calculates shifted one
    factor = 2**(nhalftones / 12.0)
    fmin_shifted = fmin * speed_up_factor * factor
    fmax_shifted = fmax * speed_up_factor * factor

    print("  initial max frequency: ",fmax * speed_up_factor)
    print("  audio max frequency  : ",audio_highest_freq)
    print("")
    print("  half tone shift : ",nhalftones)
    print("  shifted fmin : ",fmin_shifted)
    print("  shifted fmax : ",fmax_shifted)
    print("")

    # pitch window, pad
    window_size = 2**13
    pad = 2**11

    dat_shifted = shift_pitch(tr_pitched.data, nhalftones, window_size, pad)

    # normalizes
    dat_shifted = dat_shifted / np.max(np.abs(dat_shifted))

    # updates trace
    tr_pitched.data = dat_shifted

    # file output
    filename = station_name + "sonic.pitched.wav"
    tr_pitched.write(filename, format='WAV', width=4, rescale=True, framerate=audio_sampling_rate)
    print("  audio file: ",filename)
    print("")

    return tr_pitched


def create_audio_augmented(tr,speed_up_factor,fmin,fmax,t0,station_name):
    """
    creates an audio with signal augmentation
    """
    global nbandpass_freq,audio_lowest_freq,audio_highest_freq

    print("creating augmented audio:")
    print("  using signal augmentation")

    # Frequency domain
    FT_all = np.fft.rfft(tr.data)
    # amplitude spectrum
    FT_all = np.abs(FT_all)
    # Frequency axis for plotting
    FT_all_freqs = np.fft.rfftfreq(tr.stats.npts, d=tr.stats.delta)
    # gets index of maximum
    result = np.where(FT_all == np.amax(FT_all))
    idx = result[0][0]

    #print(FT_all)
    #print(FT_all_freqs)
    #print(result)

    fmax_ft = FT_all_freqs[idx]
    #if fmax_ft == 0.0: fmax_ft = fmax

    #plot_spectrum(tr)
    # normalizes
    #FT_dat = FT_dat / (np.max(np.abs(FT_dat)))
    print("  amplitude spectrum:")
    print("  length : ",len(FT_all),len(FT_all_freqs))
    print("  frequencies min/max: ",FT_all_freqs.min(),FT_all_freqs.max())
    print("  spectrum max: ",FT_all.max(),"at ",idx,fmax_ft)
    print("")
    if 1 == 0:
        plt.title("FFT raw data",size=10)
        plt.loglog(FT_all_freqs, FT_all,color='maroon')
        plt.xlim(1e-6, 1)
        plt.xlabel("Frequency [Hz]",size=8)
        plt.show()

    # initial periods
    T_min = 1.0/fmax
    T_max = 1.0/fmin

    print("  filtering: ")
    print("  minimum period = ",T_min," frequency = ",1.0/T_min)
    print("  maximum period = ",T_max," frequency = ",1.0/T_max)
    print("")

    # frequency range
    # for minimum frequency, the period is the same as the trace length.
    # puts the maximum period at half of the duration
    # such that we can nicely taper the trace to avoid aliasing effects
    T_max = T_min + 0.5 * (T_max - T_min)
    print("  maximum period for bandpassing = ",T_max," frequency = ",1.0/T_max)

    fmax = 1.0 / T_min
    fmin = 1.0 / T_max

    # trace for filtering
    # taper (same as in SAC) to avoid aliasing when cutoff
    tr_filt = tr.copy()
    tr_filt.taper(max_percentage=0.05)
    #tr_filt.plot()

    # new trace with audio sampling rate
    tr_audio = tr.copy()
    tr_audio.interpolate(sampling_rate=audio_sampling_rate/speed_up_factor)

    # new number of points and sampling rate
    npts_audio = tr_audio.stats.npts
    samp_audio = tr_audio.stats.sampling_rate

    time_audio = np.arange(0, npts_audio/samp_audio, 1.0/samp_audio)
    time_audio += t0

    # initializes data
    dat_audio = np.zeros(npts_audio,dtype=float)

    fmin0 = fmin
    fmax0 = fmax

    frange = fmax - fmin
    freq_delta = frange/nbandpass_freq

    print("  bandpass filtering:")
    print("  frequency range: ",frange)
    print("  frequency delta: ",freq_delta)
    print("")

    # use different taper length depending on center period
    taper_per_period = False

    # scales signal with signal strength from spectrum (normalized)
    amplify_by_spectrum = True

    # boost low frequencies
    low_boost = True

    # initial trace
    if not taper_per_period:
        # envelope
        tr_tmp = tr_filt.copy()
        # Envelope of filtered data
        dat_envelope = envelope(tr_tmp.data)
        print("  envelope min/max = ",dat_envelope.min(),dat_envelope.max())
        # envelope trace with audio sampling
        tr_tmp.data = dat_envelope
        tr_tmp.interpolate(sampling_rate=audio_sampling_rate/speed_up_factor)
        # taper
        tr_tmp.taper(max_percentage=0.05)
        # envelope
        signal_envelope = tr_tmp.data.copy()

    # loops over bandpass filters
    for i in range(nbandpass_freq):
        # center frequency
        freq_c = fmin0 + i / nbandpass_freq * frange + 0.5 * freq_delta

        fmin = freq_c - 0.5 * freq_delta
        fmax = freq_c + 0.5 * freq_delta

        print(i,"/",nbandpass_freq,"  filter:")
        print("  frequency center: ",freq_c," center period   : ",1.0/freq_c)
        print("  filter frequency min/max = ",fmin,fmax," period: min/max = {:.2f} / {:.2f}".format(1.0/fmax,1.0/fmin))

        # tapers initial trace
        if taper_per_period:
            # taper according to period
            period = 1.0/freq_c
            # percentage with respect to trace duration
            perc = period/duration
            if perc < 0.05: perc = 0.05
            if perc > 0.3: perc = 0.3
            print("  taper percentage = ",perc)
            # bandpass filter
            tr_tmp = tr_filt.copy()
            tr_tmp.taper(max_percentage=perc)
            tr_tmp.filter('bandpass', freqmin=fmin, freqmax=fmax, corners=2, zerophase=True)
            tr_tmp.taper(max_percentage=perc)

            #tr_tmp.plot()
            # Envelope of filtered data
            dat_envelope = envelope(tr_tmp.data)
            print("  envelope min/max = ",dat_envelope.min(),dat_envelope.max())
            # envelope trace with audio sampling
            tr_env.data = dat_envelope
            tr_tmp.interpolate(sampling_rate=audio_sampling_rate/speed_up_factor)
            # taper
            tr_tmp.taper(max_percentage=0.05)
            # envelope
            signal_envelope = tr_tmp.data.copy()

        # show
        if 1 == 0 and i == 0:
            t = np.arange(0, tr_tmp.stats.npts/tr_tmp.stats.sampling_rate, 1.0/tr_tmp.stats.sampling_rate)
            plt.plot(t, tr_tmp.data, 'k')
            plt.plot(t, dat_envelope, 'b')
            plt.title(station_name)
            plt.ylabel('Filtered Data w/ Envelope')
            plt.xlabel('Time [s]')
            plt.show()

        # frequency scaled into audible range
        freq_audio = audible_frequency(freq_c,fmin0,fmax0,
                                       audio_lowest_freq,audio_highest_freq,
                                       speed_up_factor)

        print("  audible frequency before = ",freq_audio," after duration scaling = ",freq_audio * speed_up_factor)

        # generates audio signal
        signal = generate_signal(freq_audio,time_audio)

        # scales with signal strength from spectrum (normalized)
        if amplify_by_spectrum:
            signal = multiply_with_amplitude_spectrum(signal,tr_filt,freq_c,FT_all,FT_all_freqs,speed_up_factor)

        # boost low frequencies
        if low_boost:
            b = 0.5 * (2.0 - (freq_c - fmin0)/frange)
            #b = pow(b,4.5)
            print("  low boost = ",b)
            signal = b * signal

        # amplitude
        dat_audio += signal * signal_envelope

        # show
        if 1 == 0 and i == 0:
            plt.plot(time_audio, dat_audio, 'k')
            plt.plot(time_audio, tr_tmp.data, 'b') # envelope
            plt.title(station_name)
            plt.ylabel('Audio w/ Envelope')
            plt.xlabel('Time [s]')
            plt.show()
        print("")

    # normalizes
    dat_audio = dat_audio / np.max(np.abs(dat_audio))

    # updates audio trace
    tr_audio.data = dat_audio

    # file output
    filename = station_name + "sonic.augmented.wav"
    tr_audio.write(filename, format='WAV', width=4, rescale=True, framerate=audio_sampling_rate)
    print("audio file: ",filename)
    print("")

    return tr_audio

def create_sound(T_min,filename,audio_duration,show=False):
    """
    creates a sonification of a seismogram
    shifts frequencies according to desired audio duration
    """
    global nbandpass_freq,audio_lowest_freq,audio_highest_freq,audio_sampling_rate
    global add_noise,add_reverb
    global add_augmented,augmented_percentage
    global add_localization

    print("sonification:")
    print("  minimum period : ",T_min)
    print("  file           : ",filename)
    print("  audio duration : ",audio_duration)
    print("")

    # reads in file data
    # format: #time #displacement
    #dat = np.loadtxt(filename, dtype='float')

    t = np.loadtxt(filename,dtype='float',usecols=[0])   # displacement values in 2. column
    dat = np.loadtxt(filename,dtype='float',usecols=[1]) # displacement values in 2. column

    # number of points
    npts = len(t)

    # start time
    t0 = t[0]

    # time increment
    if npts > 1:
        deltat = t[2] - t[1]
    else:
        deltat = 1.0

    # trace duration
    duration = npts * deltat

    # specfem file: **network**.**station**.**comp**.sem.ascii
    basename = os.path.basename(filename)
    names = str.split(basename,".")
    station_name = "{}.{}.{}.".format(names[0],names[1],names[2])

    #print("names: ",names)

    print("  network        : ",names[0])
    print("  station        : ",names[1])
    print("  channel        : ",names[2])
    print("  trace length   : ",len(t))
    print("  start time     : ",t0)
    print("  time           : min/max = {} / {}".format(t.min(),t.max()))
    print("  time increment :  delta t = ",deltat)
    print("  duration       : ",duration)
    print("")
    print("  data           : min/max = {:e} / {:e}".format(dat.min(),dat.max()))
    print("")

    # limits trace to start at time 0 (similar as movies, for better synchronization)
    cut_trace_at_zero_time = True
    if cut_trace_at_zero_time and t0 < 0.0:
        len_pts_tzero = int( -t0 / deltat )
        # check with time stamp, take next time step if still at negative time
        if t[len_pts_tzero] < 0.0: len_pts_tzero += 1
        # new data trace
        dat_tmp = np.copy(dat[len_pts_tzero:])
        dat = dat_tmp
        # new time line
        t_tmp = np.copy(t[len_pts_tzero:])
        t = t_tmp
        # new start time, length, duration
        t0 = t[0]
        npts = len(dat)
        duration = npts * deltat
        print("cut at approximately zero start time:")
        print("  new start time  : ",t0)
        print("  new trace length: ",npts)
        print("")

    # store as obspy trace
    tr = obspy.Trace()

    # sets trace stats
    tr.stats.network = names[0]
    tr.stats.station = names[1]
    tr.stats.channel = names[2]
    tr.stats.delta = deltat

    tr.stats.location = "00"
    tr.stats._format = "specfem ascii"
    tr.stats.ascii = {u'unit': u'M'}
    tr.stats.mseed = {u'dataquality': u'B'}

    tr.data = dat
    # arbitrary starttime
    tr.stats.starttime = obspy.UTCDateTime(2020, 1, 1, 1, 0, 0)
    tr.stats.starttime += t0

    # checks trace
    tr.verify()

    # original trace
    tr_org = tr.copy()

    #if tr.stats.sampling_rate > audio_sampling_rate:
    #    # down sample
    #    print("down sampling from {} to audio sampling rate {}".format(tr.stats.sampling_rate,audio_sampling_rate))
    #    tr.interpolate(sampling_rate=audio_sampling_rate/speed_up_factor)

    # trace info
    print(tr.stats)
    print("")
    #tr.plot()

    #sampling rate
    print("  sampling rate : ",tr.stats.sampling_rate)

    # total duration
    duration = tr.stats.delta * tr.stats.npts

    # maximum period of signal
    T_max =  duration

    # maximum frequency Nyquist
    fmax = 1.0/2.0 * 1.0/tr.stats.delta

    # minimum frequency given by trace length
    fmin = 1.0 / T_max

    print("  frequency      : min/max = {} / {}".format(fmin,fmax))
    print("  periods        : min/max = {} / {}".format(1.0/fmax,1.0/fmin))
    print("")

    # factor to scale duration from file duration to desired audio duration
    speed_up_factor = duration / audio_duration

    print("hearing range:")
    print("  frequency   minimum   = ",audio_lowest_freq," maximum = ",audio_highest_freq)
    print("  audio sampling rate   = ",audio_sampling_rate)
    print("  duration speed up scale factor = ",speed_up_factor)
    print("")

    # trace with pitch shifted
    tr_audio = create_audio_pitched(tr,speed_up_factor,fmin,fmax,station_name)

    # audio waveform
    dat_audio = tr_audio.data.copy()

    # adds trace with signal augmentation
    if add_augmented:
        tr_augmented = create_audio_augmented(tr,speed_up_factor,fmin,fmax,t0,station_name)
        # merges signals
        dat_audio += augmented_percentage * tr_augmented.data.copy()

    # scales to +/- 1
    dat_audio = dat_audio / np.max(np.abs(dat_audio))

    # slightly dim
    dat_audio *= 0.8

    # adds reverb
    if add_reverb:
        print("adding reverb")
        dat_audio = reverb_filter(dat_audio,tr_audio.stats.sampling_rate)

    if 1 == 0:
        # Frequency domain
        FT = np.fft.rfft(dat_audio)
        # amplitude spectrum
        FT = np.abs(FT)
        # Frequency axis for plotting
        FT_freqs = np.fft.rfftfreq(tr_audio.stats.npts, d=tr_audio.stats.delta)

        # gets index of maximum
        #result = np.where(FT == np.amax(FT))
        #idx = result[0]
        #fmax = FT_freqs[idx]
        # normalizes
        #FT = FT / (np.max(np.abs(FT)))
        plt.title("audio data",size=10)
        plt.loglog(FT_freqs, FT,color='maroon')
        plt.xlim(1e-6, 1)
        plt.xlabel("Frequency [Hz]",size=8)
        plt.show()

    # adds random noise
    if add_noise:
        print("adding noise")
        dat_audio = noise_filter(dat_audio)

    # obspy audio trace
    tr_audio.data = dat_audio

    # plot with initial data
    save_figure = True
    if save_figure:
        tr_tmp = tr_audio.copy()
        tr_tmp.interpolate(sampling_rate=tr.stats.sampling_rate)
        # trim to have same number of points (interplation above might have shortened by a point or so)
        #print(tr.stats)
        t1 = tr.stats.starttime
        t2 = tr.stats.endtime
        tr_tmp.trim(t1,t2,pad=True,fill_value=0.0)
        # tapered trace
        tr_filt = tr.copy()
        tr_filt.taper(max_percentage=0.05)
        dat_envelope = envelope(tr_filt.data)

        # original trace
        plt.clf()
        plt.plot(t, (tr.data), 'k')  # original
        plt.title(station_name)
        plt.ylabel('displacement')
        plt.xlabel('Time [s]')
        #plt.show()
        filename = station_name + "sonic.original.png"
        plt.savefig(filename) #, bbox_inches='tight')
        print("image file: ",filename)

        # all traces
        plt.clf()
        plt.plot(t, tr_tmp.data, 'b') # audio
        plt.plot(t, (tr.data/np.max(np.abs(tr.data))), 'k')  # scaled original
        plt.plot(t, (dat_envelope/np.max(np.abs(dat_envelope))), 'r')  # envelope
        plt.title(station_name)
        plt.ylabel('Audio')
        plt.xlabel('Time [s]')
        #plt.show()
        filename = station_name + "sonic.png"
        plt.savefig(filename) #, bbox_inches='tight')
        print("image file: ",filename)

    # for audio file output, we want to scale the duration from the true seismogram duration
    # down to the desired output file duration.
    # we thus adjusted the sampling rate accordingly with a speed_up_factor:
    #  > tr_tmp.interpolate(sampling_rate=audio_sampling_rate/speed_up_factor)
    # and use the audio sampling rate for file outputs.
    # this will shorten trace duration by the speed_up_factor.

    # file output
    filename_audio = station_name + "sonic.wav"
    tr_audio.write(filename_audio, format='WAV', width=4, rescale=True, framerate=audio_sampling_rate)
    print("audio file: ",filename_audio)

    # raw data output as wav-file
    # scales to +/- 1
    # mono
    #dat_wav = np.int16( dat_audio)
    #wavfile.write("trace_sonic.stereo.wav", audio_sampling_rate, dat_audio)

    dat_audioL = tr_audio.data.copy()
    dat_audioR = tr_audio.data.copy()

    if add_localization:
        # shifts one array to "move" sound source to one side
        if 1 == 0:
            t_shift = 1.0 # s
            nshift = int( t_shift / tr_audio.stats.delta)
            print("audio shift: ",nshift, t_shift, nshift / tr_audio.stats.sampling_rate)
            dat_audioR = shift_array(dat_audioR,nshift)
            #dat_audioR *= 0.9

        # dims one array to "move" sound source
        dat_audioR = 0.5 * dat_audioR
        dat_audioL = 0.9 * dat_audioL

    # file output
    # stereo
    dat_wav = np.vstack((dat_audioL, dat_audioR))
    dat_wav = dat_wav.transpose()

    filename = station_name + "sonic.stereo.wav"
    wavfile.write(filename, audio_sampling_rate, dat_wav)
    print("audio file: ",filename)
    print("")
    print("Done")


def usage():
    print("usage: ./run_create_sound.py T_min[e.g.,minimum period ] file(e.g., XA.S16.MXZ.sem.ascii) duration")
    print("  with")
    print("    T_min    - minimum period in s  (e.g., 13.0)")
    print("    file     - trace filename       (e.g., XA.S16.MXZ.sem.ascii)")
    print("    duration - output audio duration in s (e.g., 20.0)")


if __name__ == '__main__':
    # gets arguments
    if len(sys.argv) != 4:
        usage()
        sys.exit(1)
    else:
        T_min = float(sys.argv[1])
        filename = sys.argv[2]
        audio_duration = float(sys.argv[3])

    create_sound(T_min,filename,audio_duration)
