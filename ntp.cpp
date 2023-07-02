#include <WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <TimeLord.h>

WiFiUDP Udp;
TimeLord timeLord;

const int NTP_PACKET_SIZE = 48;     // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming & outgoing packets
static const char ntpServerName[] = "europe.pool.ntp.org";
const unsigned int localPort = 8888; // local port to listen for UDP packets
const float LONGITUDE = -8.650020;
const float LATITUDE = 42.859009;
const byte CET = 1;
const byte CEST = 2;

static byte timeZone = CET;
static int sunRiseCustom;
static int sunSetOffset;
static int sunRiseMinutes = 0;
static int sunSetMinutes = 0;

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

int getLastSunday(int year, int month)
{
    // Adjust the month and year if it's January or February
    if (month < 3)
    {
        month += 12;
        year--;
    }

    // Calculate the day of the week (0 = Saturday, 1 = Sunday, ..., 6 = Friday)
    int dayOfWeek = (1 + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;

    // Calculate the number of days to subtract to get the last Sunday
    int daysToSubtract = (dayOfWeek + 6) % 7;

    // Calculate the last Sunday of the month
    int lastSunday = 31 - daysToSubtract;

    return lastSunday;
}

bool isEuropeanSummerTime(int year, int month, int day)
{
    Serial.println("IsEuropeanSummerTime debug: ");

    if (month < 3 || month > 10)
    {
        // We are outside the range of possible European DST months
        return false;
    }

    // Calculate the last Sunday in March
    int lastSundayInMarch = getLastSunday(year, 3);

    // Calculate the last Sunday in October
    int lastSundayInOctober = getLastSunday(year, 10);

    if ((month == 3 && day >= lastSundayInMarch) ||
        (month == 10 && day < lastSundayInOctober) ||
        (month > 3 && month < 10))
    {
        // We are within the European DST range
        return true;
    }

    // We are outside the European DST range
    return false;
}

byte getTimeZone()
{
    byte today[] = {second(), minute(), hour(), day(), month(), year()};

    if (isEuropeanSummerTime(year(), month(), day()))
    {
        return CEST;
    }

    return CET;
}

void setupNtp()
{
    Serial.print("IP number assigned by DHCP is ");
    Serial.println(WiFi.localIP());
    Serial.println("Starting UDP");
    Udp.begin(localPort);
    Serial.println("waiting for sync");
    setSyncProvider(getNtpTime);
    setSyncInterval(300);

    timeLord.TimeZone(CET * 60);
    timeLord.Position(LATITUDE, LONGITUDE);
}

time_t getNtpTime()
{
    IPAddress ntpServerIP; // NTP server's ip address

    while (Udp.parsePacket() > 0)
        ; // discard any previously received packets
    Serial.println("Transmit NTP Request");
    // get a random server from the pool
    WiFi.hostByName(ntpServerName, ntpServerIP);
    Serial.print(ntpServerName);
    Serial.print(": ");
    Serial.println(ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();

    // TODO: this could be sustituted by a timer?
    // Because now is an stopper
    while (millis() - beginWait < 1500)
    {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE)
        {
            Serial.println("Receive NTP Response");
            Udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
        }
    }
    Serial.println("No NTP Response :-(");
    return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum, or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); // NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}

bool isNight()
{
    timeZone = getTimeZone();
    timeLord.TimeZone(timeZone * 60);
    byte today[] = {second(), minute(), hour(), day(), month(), year()};

    if (timeLord.SunRise(today)) // if the sun will rise today (it might not, in the [ant]arctic)
    {
        Serial.print("Sunrise: ");
        Serial.print((int)today[tl_hour]);
        Serial.print(":");
        Serial.println((int)today[tl_minute]);

        sunRiseMinutes = sunRiseCustom > 0 ? sunRiseCustom : (int)today[tl_hour] * 60 + (int)today[tl_minute];
    }

    if (timeLord.SunSet(today))
    {
        Serial.print("Sunset: ");
        Serial.print((int)today[tl_hour]);
        Serial.print(":");
        Serial.println((int)today[tl_minute]);

        sunSetMinutes = (int)today[tl_hour] * 60 + (int)today[tl_minute] - sunSetOffset;
    }

    int currentMinutes = hour() * 60 + minute();

    return currentMinutes > sunSetMinutes ||
           currentMinutes < sunRiseMinutes;
}

void setSunSetOffset(int offset)
{
    sunSetOffset = offset;
}

void setSunRiseHour(int minutes){
    sunRiseCustom = minutes;
}

String getSunRiseAndSunSetCalculated(){
    return "Sunrise: " + String(sunRiseMinutes) + " Sunset: " + String(sunSetMinutes);
}

String getDateAndHour()
{
    return String(day()) + "/" + String(month()) + "/" +
           String(year()) + " " + String(hour()) + ":" +
           String(minute()) + ":" + String(second());
}