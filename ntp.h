void setupNtp();
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
bool isNight();
String getDateAndHour();
byte getTimeZone();