void setupNtp();
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
bool isNight();
String getDateAndHour();
byte getTimeZone();
void setSunSetOffset(int offset);
void setSunRiseHour(int minutes);
String getSunRiseAndSunSetCalculated();