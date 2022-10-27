#!/usr/bin/env python

from geopy.geocoders import Nominatim
geolocator = Nominatim(user_agent='myapplication')

location = geolocator.geocode("Oslo")
lon = location.longitude
lat = location.latitude

cities = [
  "Paris",
  "Oslo",
  "Kinshasa",
  "Pretoria",
  "Istanbul",
  "Cairo",
  "Damascus",
  "Moskva",
  "Mogadishu",
  "Riyadh",
  "Tehran",
  "Astana",
  "Islamabad",
  "New Delhi",
  "Kathmandu",
  "Bangkok",
  "Kuala Lumpur",
  "Hanoi",
  "Jakarta",
  "Beijing",
  "Manila",
  "Darwin",
  "Tokyo",
  "Canberra",
  "Wellington",
  "Honolulu",
  "Papeete",
  "Fairbanks",
  "San Francisco",
  "Mexico",
  "Havana",
  "Panama",
  "Quito",
  "New York",
  "Kingstown",
  "Buenos Aires",
  "Montevideo",
  "Brasilia",
  "Reykjavik",
  "Casablanca"
]

for name in cities:
    location = geolocator.geocode(name)
    lon = location.longitude
    lat = location.latitude
    if lon < 0.0: lon += 360.0

    print("  {  %6.2f,%6.2f,\"%s\" }," % (lon,lat,name))



