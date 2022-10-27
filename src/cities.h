/*-----------------------------------------------------------------------
  shakeMovie

  originally written by Santiago v Lombeyda, Caltech, 11/2006

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
-----------------------------------------------------------------------*/

// cities.h
#ifndef CITIES_H
#define CITIES_H

typedef struct {
  float lon;  // (true longitude + 180 degrees) modulo 360 degrees
  float lat;
  const char *name;
} CityRecordType;

//  ncities = grep \" cities.h | wc -l

// constants

// city box padding
const int cityNamePaddingW = 1;  // needs at least 1, otherwise shadow on text will be cut off
const int cityNamePaddingH = 20;

// radian fraction of pi/2 to cutoff city display
const double cityCutoffDistanceRad = 0.77;

// box shadow
const bool addSmoothShadow = true;
const bool addConnectionLine = true;
const bool addCityOpacity = true;

// checks overlapping city boxes for rendering
static bool cityBoundingBoxesChecked = false;

// factor for allocation of checking buffer
static int    cityBoundingBoxFactor  =  1;
static double cityBoundingBoxFactorD = (double)cityBoundingBoxFactor;

/* ----------------------------------------------------------------------------------------------- */

// earth
int ncities_earth_all = 202;

// format:
// #lon, #lat, #name
CityRecordType cities_earth_all[] = {
  {    0.00, 90.00,"North Pole" },
  {    0.00,-90.00,"South Pole" },
  {    1.20,  6.09,"Lome" },
  {    1.32, 42.31,"Andorra la Vella" },
  {    2.06, 13.27,"Niamey" },
  {    2.20, 48.50,"Paris" },
  {    2.42,  6.23,"Porto-Novo" },
  {    3.08, 36.42,"Algiers" },
  {    4.21, 50.51,"Brussels" },
  {    4.54, 52.23,"Amsterdam" },
  {    6.09, 49.37,"Luxembourg" },
  {    6.39,  0.10,"Sao Tome" },
  {    7.28, 46.57,"Bern" },
  {    7.32,  9.05,"Abuja" },
  {    8.50,  3.45,"Malabo" },
  {    9.26,  0.25,"Libreville" },
  {    9.31, 47.08,"Vaduz" },
  {   10.11, 36.50,"Tunis" },
  {   10.45, 59.55,"Oslo" },
  {   11.35,  3.50,"Yaounde" },
  {   12.29, 41.54,"Rome" },
  {   12.30, 43.55,"San Marino" },
  {   12.34, 55.41,"Copenhagen" },
  {   13.07, 32.49,"Tripoli" },
  {   13.15, -8.50,"Luanda" },
  {   13.25, 52.30,"Berlin" },
  {   14.22, 50.05,"Prague" },
  {   14.31, 35.54,"Valletta" },
  {   14.33, 46.04,"Ljubljana" },
  {   14.59, 12.10,"N'Djamena" },
  {   15.12, -4.09,"Brazzaville" },
  {   15.15, -4.20,"Kinshasa" },
  {   15.58, 45.50,"Zagreb" },
  {   16.22, 48.12,"Vienna" },
  {   17.04,-22.35,"Windhoek" },
  {   17.07, 48.10,"Bratislava" },
  {   18.03, 59.20,"Stockholm" },
  {   18.26, 43.52,"Sarajevo" },
  {   18.35,  4.23,"Bangui" },
  {   19.05, 47.29,"Budapest" },
  {   19.49, 41.18,"Tirane" },
  {   20.37, 44.50,"Belgrade" },
  {   21.00, 52.13,"Warsaw" },
  {   21.26, 42.01,"Skopje" },
  {   23.20, 42.45,"Sofia" },
  {   23.46, 37.58,"Athens" },
  {   24.08, 56.53,"Riga" },
  {   24.48, 59.22,"Tallinn" },
  {   25.03, 60.15,"Helsinki" },
  {   25.19, 54.38,"Vilnius" },
  {   25.57,-24.45,"Gaborone" },
  {   26.10, 44.27,"Bucuresti" },
  {   27.30, 53.52,"Minsk" },
  {   27.30,-29.18,"Maseru" },
  {   28.12,-25.44,"Pretoria" },
  {   28.16,-15.28,"Lusaka" },
  {   28.50, 47.02,"Chisinau" },
  {   29.18, -3.16,"Bujumbura" },
  {   30.04, -1.59,"Kigali" },
  {   30.28, 50.30,"Kiev" },
  {   31.02,-17.43,"Harare" },
  {   31.06,-26.18,"Mbabane" },
  {   31.14, 30.01,"Cairo" },
  {   32.30,  0.20,"Kampala" },
  {   32.32,-25.58,"Maputo" },
  {   32.35, 15.31,"Khartoum" },
  {   32.54, 39.57,"Ankara" },
  {   33.25, 35.10,"Nicosia" },
  {   33.48,-14.00,"Lilongwe" },
  {   35.12, 31.47,"Jerusalem" },
  {   35.31, 33.53,"Beirut" },
  {   35.45, -6.08,"Dodoma" },
  {   35.52, 31.57,"Amman" },
  {   36.18, 33.30,"Damascus" },
  {   36.48, -1.17,"Nairobi" },
  {   37.35, 55.45,"Moskva" },
  {   38.42,  9.02,"Addis Ababa" },
  {   38.55, 15.19,"Asmara" },
  {   42.20, 11.08,"Djibouti" },
  {   43.16,-11.40,"Moroni" },
  {   44.30, 33.20,"Baghdad" },
  {   44.31, 40.10,"Yerevan" },
  {   44.50, 41.43,"T'bilisi" },
  {   45.14,-12.48,"Mamoudzou" },
  {   45.25,  2.02,"Mogadishu" },
  {   46.42, 24.41,"Riyadh" },
  {   47.31,-18.55,"Antananarivo" },
  {   48.00, 29.30,"Kuwait" },
  {   49.56, 40.29,"Baku" },
  {   50.30, 26.10,"Manama" },
  {   51.30, 35.44,"Tehran" },
  {   51.35, 25.15,"Doha" },
  {   54.22, 24.28,"Abu Dhabi" },
  {   57.30,-20.10,"Nouakchott" },
  {   57.50, 38.00,"Ashgabat" },
  {   58.36, 23.37,"Masqat" },
  {   68.48, 38.33,"Dushanbe" },
  {   69.10, 41.20,"Tashkent" },
  {   69.11, 34.28,"Kabul" },
  {   71.30, 51.10,"Nur-Sultan" }, // formerly known as: Astana
  {   73.10, 33.40,"Islamabad" },
  {   73.28,  4.00,"Male" },
  {   74.46, 42.54,"Bishkek" },
  {   77.13, 28.37,"New Delhi" },
  {   85.20, 27.45,"Kathmandu" },
  {   89.45, 27.31,"Thimphu" },
  {   90.26, 23.43,"Dhaka" },
  {   96.20, 16.45,"Yangon" },
  {  100.35, 13.45,"Bangkok" },
  {  101.41,  3.09,"Kuala Lumpur" },
  {  102.36, 17.58,"Vientiane" },
  {  104.55, 11.33,"Phnom Penh" },
  {  105.55, 21.05,"Hanoi" },
  {  106.49, -6.09,"Jakarta" },
  {  113.33, 22.12,"Macau" },
  {  115.00,  4.52,"Bandar Seri Begawan" },
  {  116.20, 39.55,"Beijing" },
  {  121.03, 14.40,"Manila" },
  {  125.30, 39.09,"P'yongyang" },
  {  125.34, -8.29,"Dili" },
  {  126.58, 37.31,"Seoul" },
  {  134.28,  7.20,"Koror" },
  {  139.75, 35.67,"Tokyo" },
  {  145.45, 15.12,"Saipan" },
  {  147.08, -9.24,"Port Moresby" },
  {  149.08,-35.15,"Canberra" },
  {  158.09,  6.55,"Palikir" },
  {  159.57, -9.27,"Honiara" },
  {  166.30,-22.17,"Noumea" },
  {  168.18,-17.45,"Port-Vila" },
  {  168.43,-45.20,"Kingston" },
  {  173.00,  1.30,"Tarawa" },
  {  174.46,-41.19,"Wellington" },
  {  178.30,-18.06,"Suva" },
  {  179.13, -8.31,"Funafuti" },
  {  186.00,-21.10,"Nuku'alofa" },
  {  188.50,-13.50,"Apia" },
  {  189.57,-14.16,"Pago Pago" },
  {  210.66,-17.32,"Papeete" },
  {  260.90, 19.20,"Mexico City" },
  {  269.78, 14.40,"Guatemala" },
  {  270.90, 13.40,"San Salvador" },
  {  271.70, 17.18,"Belmopan" },
  {  272.86, 14.05,"Tegucigalpa" },
  {  273.80, 12.06,"Managua" },
  {  275.98,  9.55,"San Jose" },
  {  277.78, 23.08,"Havana" },
  {  278.76, 19.20,"George Town" },
  {  280.75,  9.00,"Panama" },
  {  281.65, -0.15,"Quito" },
  {  282.80, 25.05,"Nassau" },
  {  282.98, 39.91,"Washington DC" },
  {  283.00,-12.00,"Lima" },
  {  283.50, 18.00,"Kingston" },
  {  284.58, 45.27,"Ottawa" },
  {  286.00,  4.34,"Bogota" },
  {  287.80, 18.40,"Port-au-Prince" },
  {  289.60,-33.24,"Santiago" },
  {  289.98, 12.32,"Oranjestad" },
  {  290.41, 18.30,"Santo Domingo" },
  {  291.00, 12.05,"Willemstad" },
  {  291.90,-16.20,"La Paz" },
  {  293.45, 10.30,"Caracas" },
  {  293.93, 18.28,"San Juan" },
  {  295.44, 18.21,"Charlotte Amalie" },
  {  295.63, 18.27,"Road Town" },
  {  297.57, 17.17,"Basseterre" },
  {  298.52, 17.20,"Saint John's" },
  {  298.56, 16.00,"Basse-Terre" },
  {  298.76, 15.20,"Roseau" },
  {  298.90, 13.10,"Kingstown" },
  {  298.98, 14.36,"Fort-de-France" },
  {  299.42, 14.02,"Castries" },
  {  300.00,-36.30,"Buenos Aires" },
  {  300.49,-51.40,"Stanley" },
  {  300.70, 13.05,"Bridgetown" },
  {  301.88,  6.50,"Georgetown" },
  {  302.70,-25.10,"Asuncion" },
  {  303.88, 46.46,"Saint-Pierre" },
  {  303.89,-34.50,"Montevideo" },
  {  304.90,  5.50,"Paramaribo" },
  {  307.82,  5.05,"Cayenne" },
  {  308.65, 64.10,"Nuuk" },
  {  312.45,-15.47,"Brasilia" },
  {  336.66, 15.02,"Praia" },
  {  338.43, 64.10,"Reykjavik" },
  {  342.71, 14.34,"Dakar" },
  {  343.60, 13.28,"Banjul" },
  {  344.55, 11.45,"Bissau" },
  {  346.51,  9.29,"Conakry" },
  {  346.83,  8.30,"Freetown" },
  {  349.53,  6.18,"Monrovia" },
  {  350.90, 38.42,"Lisbon" },
  {  352.45, 12.34,"Bamako" },
  {  353.44, 62.05,"Torshavn" },
  {  353.85, 53.21,"Dublin" },
  {  354.83,  6.49,"Yamoussoukro" },
  {  356.55, 40.25,"Madrid" },
  {  357.67, 49.26,"St. Peter Port" },
  {  358.70, 12.15,"Ouagadougou" },
  {  359.94,  5.35,"Accra" },
  {  359.95, 51.36,"London" }
};


// main subset
//
// chosen such that city labels don't overlap too much
//
// lat/lon: e.g. seach by https://www.latlong.net/search.php?keyword=casablanca
//
// here: locations from script print_cities_latlon.py, which uses geopy

int ncities_earth_main = 35;

CityRecordType cities_earth_main[] = {
  {    2.35, 48.86,"Paris" },
  {   10.74, 59.91,"Oslo" },
  {   15.31, -4.32,"Kinshasa" },
  {   28.19,-25.75,"Pretoria" },
  {   28.97, 41.01,"Istanbul" },
  {   31.24, 30.05,"Cairo" },
  {   37.62, 55.75,"Moskva" },
  {   45.34,  2.04,"Mogadishu" },
  {   46.72, 24.63,"Riyadh" },
  //{   71.43, 51.13,"Nur-Sultan" }, // formerly known as: Astana
  {   77.20, 28.61,"New Delhi" },
  {  101.69,  3.15,"Kuala Lumpur" },
  {  105.85, 21.03,"Hanoi" },
  {  106.83, -6.18,"Jakarta" },
  {  116.39, 39.91,"Beijing" },
  {  120.98, 14.59,"Manila" },
  {  130.84,-12.46,"Darwin" },
  {  139.76, 35.68,"Tokyo" },
  {  149.10,-35.30,"Canberra" },
  {  174.78,-41.29,"Wellington" },
  {  202.15, 21.33,"Honolulu" },
  {  210.43,-17.54,"Papeete" },
  {  212.28, 64.84,"Fairbanks" },
  {  237.58, 37.78,"San Francisco" },
  {  245.93, 51.04,"Calgary" },
  {  260.87, 19.43,"Mexico City" },
  {  277.64, 23.14,"Havana" },
  {  278.69,  8.31,"Panama" },
  {  281.49, -0.22,"Quito" },
  {  286.01, 40.73,"New York" },
  {  298.77, 13.16,"Kingstown" },
  {  301.56,-34.61,"Buenos Aires" },
  {  303.81,-34.91,"Montevideo" },
  {  306.80,-10.33,"Brasilia" },
  {  338.23, 64.24,"Reykjavik" },
  {  352.38, 33.60,"Casablanca" }
};


// mars
int ncities_mars = 14;

// see also: http://www.google.com/mars

// format:
// #lon, #lat, #name
CityRecordType cities_mars[] = {
  {    0.00, 90.00,"North Pole" },
  {    0.00,-90.00,"South Pole" },
  {   70.00,-42.70,"Hellas Planitia" },
  {  133.74, 47.93,"Viking 2 Lander" },
  {  135.60,  4.50,"InSight Lander" },
// {  162.90,  8.90,"Cerberus Fossae" }, // overlaps with InSight; google.com/mars: 8.9N/162.9E
// {  137.20, -4.30,"Curiosity" }, // overlaps with InSight
// {  137.7,-5.4,"Gale Crater" }, // too close to Curiosity, text overlaps
  {  30.20, -47.50,"Proctor Crater Dunes" }, // google.com/mars: 47.5S, 30.2E
  {  147.21, 25.02,"Elysium Mons" }, // google.com/mars: 146.9E/24.8N
  {  150.40, 18.8,"Albor Tholus" },
  {  175.47, 14.57,"Mars Spirit Rover" },
  {  226.20, 18.65,"Olympus Mons" },
  {  286.00,-12.00,"Vallis Marinaris" },
  {  312.05, 22.46,"Viking 1 Lander" },
  {  326.75, 19.26,"Mars Pathfinder Rover" },
  {  316.00,-49.70,"Argyre Planitia" }
};


// moon
int ncities_moon = 21;

// see also: https://en.wikipedia.org/wiki/List_of_lunar_features

// format:
// #lon, #lat, #name
CityRecordType cities_moon[] = {
  {    0.00, 90.00,"North Pole" },
  {    0.00,-90.00,"South Pole" },
  {   59.10, 17.00,"Mare Crisium" },          // sea of crises: 17.0 N  59.1 E
  {  -38.60,-24.40,"Mare Humorum" },          // sea of moisture: 24.4 S  38.6 W
  {  147.90, 27.30,"Mare Moscoviense" },      // sea of moscow: 27.3 N 147.9 E
  {  -92.80,-19.40,"Mare Orientale" },        // eastern sea: 19.4 S 92.8 W
  // mountains
  //{   -2.86, 19.92,"Mons Huygens" },        // 19.92 N 2.86 W
  //{   36.11, 47.49,"Montes Jura" },         // 47.49 N 36.11 W
  //{  -30.40, 24.4000,"Catena Yuri" },       // 24.4 N 30.4 W
  {-158.6335, 5.4125,"Selenean summit" },     // https://en.wikipedia.org/wiki/Selenean_summit: 5.4125 N 158.6335 W
  // craters: https://en.wikipedia.org/wiki/List_of_craters_on_the_Moon
  {  173.40,-16.80,"Aitken" },            // Aitken 16.8°S 173.4°E
  {   17.40, 50.20,"Aristoteles" },       // 50.2 N 17.4 E
  //{   -1.90,-18.20,"Arzachel" },          // 18.2 S 1.9 W
  { -132.88,-34.01,"Chebyshev" },         // Chebyshev 34.01 S 132.88 W
  {  -20.08,  9.62,"Copernicus" },        // 9.62 N 20.08 W
  {  122.60,-19.30,"Fermi" },             // 19.3 S 122.6 E
  {   80.90,-27.20,"Humboldt" },          // 27.2 S 80.9 E
  {   -9.30, 51.60,"Plato" },             // 51.6 N 9.3 W
  {  -11.36,-43.31,"Tycho" },             // 43.31 S 11.36 W
  //{   54.72, 14.57,"Picard" },            // 14.57 N 54.72 E
  // aritifical objects: https://en.wikipedia.org/wiki/List_of_artificial_objects_on_the_Moon
  {  -64.37,  7.08,"Luna 9" }, // 7.08 N 64.37 W
  {  162.00,  6.70,"Lunar Orbiter 1" }, //  6.70 N 162 E
  // apollo missions seismometers:
  {  -23.42456, -3.01084,"Apollo 12" }, // STATIONS (lat/lon): S12        XA        -3.01084     -23.42456
  {  -17.47753, -3.64450,"Apollo 14" }, // STATIONS (lat/lon): S14        XA        -3.64450     -17.47753
  {    3.62981, 26.13407,"Apollo 15" }, // STATIONS (lat/lon): S15        XA        26.13407       3.62981
  {   15.49649, -8.97577,"Apollo 16" }  // STATIONS (lat/lon): S16        XA        -8.97577      15.49649
};



/* ----------------------------------------------------------------------------------------------- */

// adds city labels

/* ----------------------------------------------------------------------------------------------- */


void determineCityDistances(double latitude,
                            double longitude,
                            int ncities,
                            CityRecordType *cities,
                            float *cityDistances,
                            float *cityDistancesPixel,
                            double globe_radius_km){

  TRACE("cities: determineCityDistances")

  // view center position rotated to screen
  double lon0 = (longitude-90.0)/180.0*pi;
  double lat0 = -latitude/180.0*pi;

  // converts lat/lon to azimuth/elevation
  // longitudes: + 180 - 90 = + 90 (quarter rotation into center of screen)
  //
  //cities[i].lat = (float)(-cities[i].lat/180.0*pi);       // range flipped, -pi/2,pi/2
  //cities[i].lon = (float)((cities[i].lon-90.0)/180.0*pi); // range -pi/2,+3/2pi to shift into visible hemisphere

  //double x0,y0,z0;
  //latlon_2_xyz(lon0,lat0,&x0,&y0,&z0);

  // determine distance to view center
  for (int nth=0; nth<ncities; nth++) {
    // city position rotated to screen
    double lon1 = (cities[nth].lon-90.0)/180.0*pi;
    double lat1  = -cities[nth].lat/180.0*pi;

    // city lat/lon position
    //double clat,clon;
    //azimuthelevation_2_latlon_geo(cityAzi[nth],cityEle[nth],&clat,&clon);

    // bounds lat [-pi/2,pi/2]
    if (lat0 < -pi/2.0) lat0 = -pi/2.0f;
    if (lat0 > pi/2.0) lat0 = pi/2.0f;
    if (lat1 < -pi/2.0) lat1 = -pi/2.0f;
    if (lat1 > pi/2.0) lat1 = pi/2.0f;
    // bounds lon [-pi,pi]
    if (lon0 < -pi) lon0 += 2.0*pi;
    if (lon0 > pi) lon0 -= 2.0*pi;
    if (lon1 < -pi) lon1 += 2.0*pi;
    if (lon1 > pi) lon1 -= 2.0*pi;

    // altitude as distance to geographical point
    // haversine distance in rad
    double a = pow(sin((lat0-lat1)/2.0),2.0) + cos(lat1) * cos(lat0) * pow(sin((lon0-lon1)/2.0),2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0f - a)); // between -pi,pi
    double distance = c * globe_radius_km;

    cityDistances[nth] = (float) distance;

    // distance to view center position
    //double x1,y1,z1;
    //latlon_2_xyz(lon1,lat1,&x1,&y1,&z1);
    //double dot = x1*x0 + y1*y0 + z1*z0;
    //double theta = acos(dot);
    //double distance = theta*globe_radius_km;

    // set unreal distance if city too far away from view position (will not be rendered
    //static double maxTangentAngle = cos(70.0/180.0*pi);
    //if (dot <= maxTangentAngle) cityDistances[nth] = UNREAL_DISTANCE;

    // 0.77 includes Fairbanks even for center lat at zero...
    if (c > cityCutoffDistanceRad * pi/2.0) cityDistances[nth] = UNREAL_DISTANCE;

    // for pixel determination
    cityDistancesPixel[nth] = distance;
  }

  //debug
  //for (int nth=0; nth<ncities; nth++){
  //  std::cerr << "city distances: " << cities[nth].name  << " " << cityDistances[nth] << " km " << std::endl;
  //}
}

/* ----------------------------------------------------------------------------------------------- */

// determines closest pixel for city position

void determineCityPosition(int ncities,
                           float *cityDistances,float *cityDistancesPixel,
                           float *cityAzi,float *cityEle,
                           int * cityPositionX,int * cityPositionY,
                           int i, int j,
                           double p_azimuth,double p_elevation){

  TRACE("cities: determineCityPosition")

  // determines city distances to current pixel position
  for (int nth=0; nth<ncities; nth++){
    if (cityDistances[nth] != UNREAL_DISTANCE) {
      // note: cities[].lon has azimuth stored, cities[].lat has elevation stored
      float currentAngularDistanceToClosestCity =
        (float)(p_azimuth - cityAzi[nth])*(p_azimuth - cityAzi[nth])+
        (float)(p_elevation - cityEle[nth])*(p_elevation - cityEle[nth]);

      if (cityDistancesPixel[nth] < 0.0f || currentAngularDistanceToClosestCity < cityDistancesPixel[nth]) {
#if defined(_OPENMP)
#pragma omp atomic write
#endif
        cityDistancesPixel[nth] = currentAngularDistanceToClosestCity;
#if defined(_OPENMP)
#pragma omp atomic write
#endif
        cityPositionX[nth] = i;

#if defined(_OPENMP)
#pragma omp atomic write
#endif
        cityPositionY[nth] = j;
      }
    }
  }

  //debug
  /*
  if (i == image_w/2 && j == image_h/2){
    std::cerr << "point: azimuth = " << p_azimuth*180./pi << " elevation = " << p_elevation*180./pi << std::endl;
    for (int nth=0; nth<ncities; nth++) {
      std::cerr << "point: city = " << cityAzi[nth]*180./pi << " elevation = " << cityEle[nth]*180./pi
                << " " <<  cityDistancesPixel[nth] << std::endl;
    }
  }
  */
}



/* ----------------------------------------------------------------------------------------------- */

// quicksort routine
// modified from: https://rosettacode.org/wiki/Sorting_algorithms/Quicksort#C

void quicksort(float *A, int *Indx, int len) {
  if (len < 2) return;

  float pivot = A[len / 2];

  int i, j;
  for (i = 0, j = len - 1; ; i++, j--) {
    while (A[i] < pivot) i++;
    while (A[j] > pivot) j--;

    if (i >= j) break;

    float temp = A[i];
    A[i]     = A[j];
    A[j]     = temp;

    int itemp = Indx[i];
    Indx[i]     = Indx[j];
    Indx[j]     = itemp;
  }
  quicksort(A, Indx, i);
  quicksort(A + i, Indx + i, len - i);
}

/* ----------------------------------------------------------------------------------------------- */


void addCitiesToImage(unsigned char* imagebuffer,
                      int image_w,
                      int image_h,
                      int ncities,
                      CityRecordType *cities,
                      int *cityCloseness,
                      float *cityDistances,
                      int *cityPositionX,
                      int *cityPositionY,
                      unsigned char * cityBoundingBoxes,
                      bool create_halfimage,
                      unsigned char *halfimagebuffer,
                      float *halfCityDistances,
                      unsigned char * halfCityBoundingBoxes,
                      bool rotateglobe,
                      double globe_radius_km,
                      unsigned char *textColor,
                      bool verbose=false,
                      int boldfactor=1){

  TRACE("cities: addCitiesToImage")

  int closestCityPosX = 0;
  int closestCityPosY = 0;

  // small image picture
  int halfWidth  = image_w/2;
  int halfHeight = image_h/2;

  int cityBoundingBoxWidth  = iceil(image_w/cityBoundingBoxFactorD);
  int cityBoundingBoxHeight = iceil(image_h/cityBoundingBoxFactorD);

  // re-evaluates overlapping city bounding boxes for each new frame
  if (rotateglobe) { cityBoundingBoxesChecked = false; }

  // TODO-NOTE: block needs to be moved to separate file, and made
  // generic for w and halfWidth. only referred in two places! but
  // for second pass all positions=/2 & cityBoundingBoxes needs to be duplicated.

  // sorts cities to have closest ones to center first
  int *tmpIndex = (int*) malloc(ncities * sizeof(int));
  float *tmpDist = (float*) malloc(ncities * sizeof(float));

  for (int in=0; in<ncities; in++) {
    int nth = cityCloseness[in]; // closest to source locations
    float dist = cityDistances[nth];  // distance to viewpoint center
    tmpIndex[in] = nth;
    tmpDist[in] = dist;
  }
  // sorts with smaller distances first
  quicksort(tmpDist, tmpIndex, ncities);

  // render each city if in window
  if (!cityBoundingBoxesChecked) {
    // clears out mask
    memset(cityBoundingBoxes,0,cityBoundingBoxWidth*cityBoundingBoxHeight);

    // half-image
    if (create_halfimage){
      // clears out mask
      memset(halfCityBoundingBoxes,0,cityBoundingBoxWidth*cityBoundingBoxHeight/4);
      // re-set distances
      if (halfCityDistances != NULL){
        for (int i=0;i<ncities; i++) halfCityDistances[i] = cityDistances[i];
      }
    }

    for (int in=0; in<ncities; in++) {
      //int nth = cityCloseness[in];
      // start with closest to center
      int nth = tmpIndex[in];

      //debug
      //std::cerr<< "debug: cities: " << cities[nth].name << " distance " << cityDistances[nth] << std::endl;

      if (cityDistances[nth] < UNREAL_DISTANCE &&
          cityPositionX[nth] > 0 && cityPositionY[nth] > 0 &&
          cityPositionX[nth] < image_w-1 && cityPositionY[nth] < image_h-1) {

        //int cityNameBufferSize   = getRenderTextBufferSizeNeeded(cities[nth].name,cityNamePaddingW,cityNamePaddingH);

        int cityNameBufferWidth  = getRenderTextBufferSizeWidth (cities[nth].name,cityNamePaddingW);
        int cityNameBufferHeight = getRenderTextBufferSizeHeight(cities[nth].name,cityNamePaddingH);

        closestCityPosX = cityPositionX[nth];
        closestCityPosY = cityPositionY[nth];

        int tooCloseInPixels = 4 + boldfactor;
        int cityw = iceil((cityNameBufferWidth*boldfactor  + tooCloseInPixels)/cityBoundingBoxFactorD);
        int cityh = iceil((cityNameBufferHeight*boldfactor + tooCloseInPixels)/cityBoundingBoxFactorD);

        bool cleanBoundingBox;

        // moves box below, to have reference marker at top-left
        if (closestCityPosY < halfHeight){
          closestCityPosY -= cityh;
        }

        // debug
        if (verbose) std::cerr<< "cities: " << cities[nth].name
                              << " position x,y = " << closestCityPosX << "," << closestCityPosY
                              << " w x h = " << cityw << "," << cityh << std::endl;

        cleanBoundingBox = boundingBoxCheck(cityw, cityh, closestCityPosX, closestCityPosY,
                                            cityBoundingBoxFactor, cityBoundingBoxWidth,
                                            cityBoundingBoxes, &cityDistances[nth],
                                            image_w, image_h);

        // debug
        if (verbose) std::cerr<< "cities: " << cities[nth].name << " clean box " << cleanBoundingBox << std::endl;

        // half-image
        if (halfCityDistances != NULL) {
          halfCityDistances[nth] = cityDistances[nth];
          if (halfCityDistances[nth] < UNREAL_DISTANCE)
            boundingBoxCheck( cityw, cityh, closestCityPosX/2, closestCityPosY/2,
                              cityBoundingBoxFactor, cityBoundingBoxWidth/2,
                              halfCityBoundingBoxes, &halfCityDistances[nth],
                              halfWidth, halfHeight);
        }
      }
    }
    cityBoundingBoxesChecked = true;
  }

  // renders cities
  for (int in=ncities-1; in>=0; in--) {
    // renders city names

    //int nth = cityCloseness[in];
    // start with closest to center
    int nth = tmpIndex[in];

    int cityNameBufferWidth = getRenderTextBufferSizeWidth(cities[nth].name,cityNamePaddingW);
    int cityNameBufferHeight = getRenderTextBufferSizeHeight(cities[nth].name,cityNamePaddingH);

    if (cityDistances[nth] < UNREAL_DISTANCE &&
        cityPositionX[nth] > 0 && cityPositionY[nth] > 0 &&
        cityPositionX[nth] + cityNameBufferWidth - cityNamePaddingW < image_w-1 &&
        cityPositionY[nth] + cityNameBufferHeight - cityNamePaddingH < image_h-1) {

      int cityNameBufferSize = getRenderTextBufferSizeNeeded(cities[nth].name,cityNamePaddingW,cityNamePaddingH);

      unsigned char *cityNameBuffer = (unsigned char *)malloc(cityNameBufferSize);
      if (cityNameBuffer == NULL){
        std::cerr << "Error. allocating cityNameBuffer. Exiting." << std::endl;
        return;
      }

      closestCityPosX = cityPositionX[nth];
      closestCityPosY = cityPositionY[nth];

      // adds marker to image buffer
      markCityOnImage(closestCityPosX, closestCityPosY, image_w, image_h, imagebuffer, verbose, boldfactor);

      // half-image
      if (halfCityDistances != NULL){
        if (halfCityDistances[nth] < UNREAL_DISTANCE     &&
            closestCityPosX+cityNameBufferWidth*2  < image_w-2 &&
            closestCityPosY+cityNameBufferHeight*2 < image_h-2 ) {
          markCityOnImage(closestCityPosX/2, closestCityPosY/2, halfWidth, halfHeight, halfimagebuffer, verbose, boldfactor/2);
        } else {
          halfCityDistances[nth] = UNREAL_DISTANCE;
        }
      }

      // renders text buffer
      renderText(cities[nth].name,cityNameBuffer,cityNamePaddingW,cityNamePaddingH);
      addShadowToRenderedText(cities[nth].name,cityNameBuffer,cityNamePaddingW,cityNamePaddingH);

      // adds shadow to box
      if (addSmoothShadow) {
        int shadowRampSideW = 2 * cityNamePaddingW + 1;
        int shadowRampSideH = 2 * cityNamePaddingH + 1;
        unsigned char *shadowRamp = (unsigned char *)calloc(shadowRampSideW*shadowRampSideH,1);
        int index = 0;

        // 1.0 for crisp border, 0 for blank row/col. 0.5 subtle?
        for (int j=0; j<shadowRampSideH; j++) {
          double valj = (abs(j-cityNamePaddingH)-1.0)/(double)(cityNamePaddingH+0.5-1.0);
          if (valj < 0.0) valj = 0;

          for (int i=0; i<shadowRampSideW; i++) {
            double vali = (abs(i-cityNamePaddingW)-1.0)/(double)(cityNamePaddingW+0.5-1.0);
            if (vali < 0.0) vali = 0;

            vali = sqrt((vali*vali)+(valj*valj))*63.0 + 1;
            if (vali >= 64.0) vali = 0.0;
            shadowRamp[index] = ((unsigned char)vali)*4;
            // debug
            //printf("debug: shadowRamp  %3i",shadowRamp[index]);
            index++;
          }
          //if (verbose) printf("\n");
        }

        // adds shadow ramp values
        for (int j=cityNamePaddingH; j<cityNameBufferHeight-cityNamePaddingH; j++) {
          index = j*cityNameBufferWidth + cityNamePaddingH;
          for (int i=cityNamePaddingW; i<cityNameBufferWidth-cityNamePaddingW; i++,index++) {
            if (cityNameBuffer[index]&1 || cityNameBuffer[index]&2) {
              for (int sj=j-cityNamePaddingH; sj<=j+cityNamePaddingH; sj++) {
                for (int si=i-cityNamePaddingW; si<=i+cityNamePaddingW; si++) {
                  int shadowfalloffindex = (sj-(j-cityNamePaddingH))*shadowRampSideW+si-(i-cityNamePaddingW);
                  int neighborindex = sj*cityNameBufferWidth+si;

                  if (shadowRamp[shadowfalloffindex]) {
                    if (cityNameBuffer[neighborindex] < 4){
                      cityNameBuffer[neighborindex] |= shadowRamp[shadowfalloffindex];
                    } else if (shadowRamp[shadowfalloffindex] < (cityNameBuffer[neighborindex]&252)) {
                      cityNameBuffer[neighborindex] &= 3;
                      cityNameBuffer[neighborindex] |= shadowRamp[shadowfalloffindex];
                    }
                  }
                }
              }
            }
            // debug
            //cityNameBuffer[index] = 255;
          }
        } // end rendering shadow index
      } // end add smooth shadow

      // connection line from city mark to city text
      if (addConnectionLine){
        // offset to avoid hitting mark sign
        int offsetx = 2;
        int offsety = 2;
        if (cityNamePaddingW < 2){ offsetx = 0; offsety = 4;}
        if (cityNamePaddingH < 2){ offsetx = 4; offsety = 0;}
        // end index
        int iend = cityNamePaddingW;
        int jend = cityNameBufferHeight - cityNamePaddingH;
        if (cityNamePaddingW < 2) iend = 2;
        if (cityNamePaddingH < 2) jend = cityNameBufferHeight - 2;

        // padding ratio
        float ratio;
        if (cityNamePaddingH > 0) ratio = (float)(cityNamePaddingW) / (float)(cityNamePaddingH);
        else ratio = 0.0;

        if (closestCityPosY < halfHeight){
          // box below, reference marker at top-left
          for (int j=0; j < cityNamePaddingH-1; j++) {
            for (int i=0; i< iend; i++) {
              // adds pixel colors
              int index = j*cityNameBufferWidth + i;
              if (i >= offsetx && j >= offsety){
                // white pixel
                if (i == (int) (j*ratio)) cityNameBuffer[index] = 255;
                // shadow pixel (sets == 2 such that bit-operation &2 will be true in rendering routine)
                if (i == (int) (j*ratio) +1) cityNameBuffer[index] = 2;
              }
            }
          }
        }else{
          // box ontop, reference marker at bottom-left
          for (int j=cityNameBufferHeight; j > jend; j--) {
            for (int i=0; i< iend; i++) {
              // reverted index (to go from bottom-left to top-right, instead of top-left to bottom-right for i == j)
              int jj = cityNameBufferHeight - j;

              // adds pixel colors
              int index = j*cityNameBufferWidth + i;
              if (i >= offsetx && jj >= offsety){
                // white pixel
                if (i == (int) (jj*ratio)) cityNameBuffer[index] = 255;
                // shadow pixel (sets == 2 such that bit-operation &2 will be true in rendering routine)
                if (i == (int) (jj*ratio) +1) cityNameBuffer[index] = 2;
              }
            }
          }
        }
      }

      // shifts text box
      //closestCityPosX += (6*boldfactor - cityNamePaddingW);
      //closestCityPosY -= (7*boldfactor + cityNamePaddingH);

      // determines opacity factor based on distance
      double opacity = 1.0;

      if (addCityOpacity){
        float distance = cityDistances[nth]; // distance in km
        // normalized to [-pi/2,pi/2]
        float c = distance / globe_radius_km;
        // range [0,1]
        c = fabs(c) / (pi/2.0);
        // transition range, cutoff by default is 0.77
        if (c > 0.7 * cityCutoffDistanceRad){
          // limit between 0.7 - 1.0 x cutoff
          if (c > cityCutoffDistanceRad) c = cityCutoffDistanceRad;
          // range [0,1]
          c = 1.0 - (c - 0.7 * cityCutoffDistanceRad)/(0.3*cityCutoffDistanceRad);
          // range [1,0] with nonlinear scaling
          c = pow(c,2.5);
          opacity = c;
        }
        //debug
        //printf("debug: opacity factor %f \n",opacity);
      }

      // moves box below, to have reference marker at top-left
      if (closestCityPosY < halfHeight){
        closestCityPosY -= (cityNameBufferHeight-1)*boldfactor;
      }

      // adds text image to image buffer
      overlayRenderedTextOnImage( closestCityPosX, closestCityPosY,
                                  cityNameBufferWidth, cityNameBufferHeight,
                                  textColor, opacity, cityNameBuffer, addSmoothShadow,
                                  image_w, image_h, imagebuffer, verbose, boldfactor);

      // half-image
      if (halfCityDistances != NULL){
        if (halfCityDistances[nth] < UNREAL_DISTANCE) {
          // re-position text
          closestCityPosX -= (6*(float)boldfactor/2 - cityNamePaddingW);
          closestCityPosX = floor(closestCityPosX/2.0f);
          closestCityPosX += (6*(float)boldfactor/2-cityNamePaddingW);

          closestCityPosY += (7*(float)boldfactor/2 + cityNamePaddingH);
          closestCityPosY = floor(closestCityPosY/2.0f);
          closestCityPosY -= (7*(float)boldfactor/2+cityNamePaddingH);

          overlayRenderedTextOnImage( closestCityPosX, closestCityPosY,
                                      cityNameBufferWidth, cityNameBufferHeight,
                                      textColor, opacity, cityNameBuffer, addSmoothShadow,
                                      halfWidth, halfHeight, halfimagebuffer, verbose);
        }
      }

      // info output
      if (verbose) std::cerr << "cities: " << cities[nth].name << " city should be in pixels " <<
                                closestCityPosX << "," << closestCityPosY << "  (" <<
                                cityDistances[nth] << " km)" << std::endl;

      free(cityNameBuffer);
    }
  } // for

  // frees temporary arrays
  free(tmpDist);
  free(tmpIndex);
}

#endif   // CITIES_H
