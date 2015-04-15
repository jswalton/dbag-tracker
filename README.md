# dbag-tracker (Proof of concept)

## tracking tandem/sport cut-aways 

- [x] Develop Device concept - Feasability study with GRPS/GPS breakout hardware - Arduino based 
- [x] Develop Server side concept - functionality to receive coordinates (lat,long)
- [ ] Drop this shit out of airplanes & Track it (fun stuff)
- [ ] Refine hardware concepts - breakway activation?
- [ ] Refine software/server concepts - secure, proprietary and central location for client device data 
- [ ] Throw it out of planes again 
- [ ] TBD

![WHAT](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcRJxJlwtgETzeSCHs6jXv5DuGiO4ENjNapZwDfp4ReU6-5dJNrrqglzTo8)


##Hardware Concept Component
In order to quickly model hardware components, many "breakout" components were used. This allows for quick proofing and feasability studies.

Hardware used: 
- [Adafruit Flora](https://learn.adafruit.com/getting-started-with-flora) (Atmega32u4) - Arduino Compatable
- [Adafruit Fona](https://learn.adafruit.com/adafruit-fona-mini-gsm-gprs-cellular-phone-module) - GRPS/SIM Module
- [Adafruit Wearable GPS](https://learn.adafruit.com/flora-wearable-gps) - GPS Module 

Most of the hardware choices were made with size considerations in mind. There is no particular strategy for types/brands.

##Software Concept Component

Keeping with the rapid prototyping paradigm a sinatra/heroku/cedar template was used to rapidly create a REST api for posting location data 
Postgres is used for data storage

