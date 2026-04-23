# MoodLink

MoodLink is a pair of smart desktop companion devices designed for people in long-distance relationships, including couples, families living apart, and international friends. It uses Wi-Fi, MQTT communication, automatic time synchronisation, a TFT emoji interface, and a 3D-printed enclosure to support passive awareness of a partner's mood, availability, and presence.

Rather than requiring a direct message or phone call, MoodLink allows users to glance at the paired device and understand whether the other person may be free, busy, sleeping, missing them, or having a bad day. This supports lightweight emotional communication and helps reduce uncertainty around when it is appropriate to reach out.

## Introduction

Living in an increasingly connected and digitised world has contributed to the growth of long-distance relationships, including couples, families living apart, and international friends (DiGiovanni et al., 2026). Although existing technologies such as phones, social media, and messaging platforms facilitate communication, differences in time zones, work schedules, and daily routines often create uncertainty about when contact is appropriate. This challenge is particularly evident in romantic relationships: Neustaedter and Greenberg (2012) found that long-distance couples often experienced feelings of disconnection because they were unable to easily sense their partner's mood, energy level, and availability, which in turn could produce anxiety not fully resolved through calls alone.

More broadly, loneliness and perceived social disconnection from significant others can negatively affect both psychological and physical wellbeing, especially when separation is prolonged by distance (Cacioppo and Patrick, 2008). At the same time, emotional presence remains an important but often unmet need in long-distance relationships (Dey and de Guzman, 2006). Research on ambient IoT further suggests that such devices can communicate information passively without demanding users' full attention (Davis et al., 2017; Hassenzahl et al., 2012). MoodLink was developed as a Kickstarter-style project in response to this gap, using ambient IoT technology to help couples, friends, and families maintain awareness of one another's feelings and availability without the fear of interrupting.

## Technical Overview

### Device System

#### Hardware

> Image placeholder: insert Figure 1 showing the hardware component table.

> Image placeholder: insert Figure 2 showing hardware wiring / system components.

MoodLink's hardware components are soldered on a perfboard to minimise the risk of disconnection, ensuring stable shared power rails and grounds. The components were chosen with the aim of facilitating passive interactions that feel tactile and deliberate.

#### Functionalities and Controlling Code

> Image placeholder: insert Figure 3 showing the welcome screen.

MoodLink displays a welcome screen that leads users to the main display, which contains the five status modes. Two additional displays were added to improve user interaction: an offline page and a checking page when a partner's connection is not yet confirmed.

> Image placeholder: insert Figure 4 showing the offline status page.

The ESP32-S3 board supports a convenient Wi-Fi setup process and keeps the interaction within the device itself without requiring a mobile app. Rather than hardcoding network credentials, we built a captive portal for initial setup, which is commonly used by commercial IoT products. Each MoodLink device connects to the internet through the ESP32-S3's built-in Wi-Fi. On first setup, the device broadcasts a temporary hotspot. When users connect their phone to it, a configuration page is served for entering the home Wi-Fi, password, and pairing code.

The ESP32's ability to scan surrounding networks was then included. A scannable list was added so users can select their Wi-Fi network more easily. Manual input is retained as a fallback for networks that do not appear in scan results. The shared pairing code is hardcoded into the device, ensuring only the intended pair of devices can communicate. The credentials are saved in the ESP32's non-volatile storage, while a Wi-Fi icon on the screen's top bar shows connection status. Devices can automatically reconnect on every reboot without repeating the setup process.

The emoji status display acts as the main communication element of MoodLink. The emojis are drawn using geometric primitives such as arcs and lines instead of font glyphs or bitmap images (lovyan03, n.d.). This allows the emojis to scale smoothly across different rendering contexts. Font glyphs lack the required expressiveness for representing mood, while bitmaps can become pixelated when scaled (Ilitek, 2013). Bitmap images would also need to be stored on the ESP32 using a SPIFFS system, which would consume additional memory (Espressif Systems, 2024a; Espressif Systems, 2024b).

A global `SCALE` value is multiplied by drawing measurements, allowing the same drawing code to render across three contexts: the schedule page, the self panel, and the partner panel. The partner panel on the left takes up most of the screen so users can easily see their partner's status, while the user's own status is kept smaller on the right.

> Image placeholder: insert Figure 5 showing busy status.

> Image placeholder: insert Figure 6 showing bad day status.

> Image placeholder: insert Figure 7 showing miss you status.

> Image placeholder: insert Figure 8 showing sleeping status.

The five statuses are kept simple but expressive to support a clear understanding of the user's mood. The miss you status is inspired by the iOS heart reaction, a familiar visual for users (Apple Inc., 2024). The sleeping status is inspired by *We Bare Bears* (Cartoon Network, 2015). To prevent tearing from redrawing the bear graphic every 90ms at parallel bus speeds, all elements apart from the Z animation are drawn once using `startSleepScene()`, with each frame only deleting the previous Z position through bounding boxes in `zPrev[i]`. The Z animation uses a non-blocking `millis()` timer so it does not block button handling, clock updates, or MQTT. To reduce visual clutter, the user's sleeping status was simplified to three Zs, while the partner's sleeping status keeps the full bear graphic.

#### Time and Schedule

For the time and scheduling function, our design combines local interaction with online services and saved settings on the ESP32-S3 board. In the early version of MoodLink, time and timezone were set manually by the rotary encoder, which was inconvenient after restarts or when used in different countries. Therefore, we added an Auto Time mode. When connected to Wi-Fi, MoodLink can obtain the UTC offset through an IP-based API and synchronise the clock with NTP to automatically update the time and timezone (Espressif Systems, n.d.-c; IP-API, n.d.). We also kept the manual option because it remains useful when the network is unavailable or users want direct control. The ESP32 saves the selected mode and timezone in Preferences so that the main settings are retained after restart (Espressif Systems, n.d.-b).

> Image placeholder: insert Figure 9 showing Auto Time mode.

> Image placeholder: insert Figure 10 showing Manual Time mode.

The schedule function was also developed step by step. Firstly, device status could only be changed manually by the rotary encoder. We extended this into a schedule system so users can plan emotional states in advance. Each schedule includes a target status, start time, end time, and a repeat rule such as once, daily, weekly, or monthly. We also added checks so invalid schedules, such as an end time earlier than the start time, cannot be saved. During use, the device checks active schedules and changes the status automatically; then it returns to the previous status afterwards. This makes time in our project not only something to display, but also a way to control the device's behaviour.

> Image placeholder: insert Figure 11 showing Schedule Function.

#### Display Configuration

Issues with pixel corruption and colour display occurred because the ILI9488 driver used in the TFT screen hardware specification did not fully match the LovyanGFX configuration. This was addressed through explicit settings in the code. `cfg.dlen_16bit = false` was important for supporting the ILI9488 colour transmission format. The parallel bus speed was adjusted to make updates more reliable. An explicit read frequency was also added to reduce unpredictable display colours. Readability was disabled using `readable = false` to prevent bus conflicts because the display's read pin is tied to 3.3V. `rgb_order = false` was added to ensure the correct colour channel order, and `bus_shared = false` was set because the display bus is not shared with other devices.

> Image placeholder: insert Figure 12 showing pixelated / corrupted free status display.

#### MQTT Communication

The two MoodLink devices communicate over MQTT, a lightweight communication protocol widely used in IoT products (Bandyopadhyay and Bhattacharyya, 2013). MQTT allows direct communication without needing to know IP addresses and supports retained status messages after device reconnection (Naik, 2017). Each device publishes to four topics: emotional status, Unix timestamp time, timezone index, and heartbeat. Status updates are transmitted immediately after user interaction.

The heartbeat was a final design improvement for online/offline indication. Each device publishes a heartbeat every five seconds regardless of user activity. If no message is received from the partner device for 130 seconds, the device marks the paired device as offline and displays the time when the partner was last seen. This ensures the connection state is always communicated to the user rather than silently failing, which is central to MoodLink's aim of providing emotional reassurance rather than uncertainty.

### Enclosure

The enclosure was developed as a communicative part of MoodLink. All models were built in Fusion 360 and 3D printed in PLA. It integrates all components and internal wiring into a compact desktop object. The screen is recessed into the upper section, while the rotary knob is placed centrally to support comfortable interaction. A knob cover unifies the overall appearance and a logo nameplate highlights the unique product identity.

> Image placeholder: insert Figure 13 showing assembled enclosure 3D models.

> Image placeholder: insert Figure 14 showing front-right upper view of all models.

> Image placeholder: insert Figure 15 showing lower-left rear view of all models.

> Image placeholder: insert Figure 16 showing knob cover.

> Image placeholder: insert Figure 17 showing logo nameplate.

The design was informed by Nabaztag, an ambient device whose rounded and character-like form showed that connected products can communicate through both presence and function (Violet, 2025). Disney's principle of appeal was referenced to guide overall friendliness (Thomas and Johnston, 1981). This informed the decision to treat the screen as a face so the device could sit naturally as a companion object.

Personalised accessories currently include a Santa hat, rabbit ears, and a Sorting Hat, extending the sense of companionship and ownership. The connection of the accessories was achieved through six magnetic points.

> Image placeholder: insert Figure 18 showing personalised accessories from left to right: Santa hat, Sorting Hat, rabbit ears.

For the iteration, an earlier prototype used a rectangular screen on a stand, which was functional but visually far from a companion device. A later sketch explored a more rounded enclosure with a clearer head-body relationship. A fully integrated shell was also considered, but a separable structure was adopted because it is easier for debugging and reassembly. These changes improved both the emotional character and the practical usability of MoodLink.

> Image placeholder: insert Figure 19 showing sketch of the first version of the enclosure.

> Image placeholder: insert Figure 20 showing sketch of the second version of the enclosure.

> Image placeholder: insert Figure 21 showing the integrated head and body version.

## How to Use

### Prerequisites

Install the Arduino IDE and add ESP32 board support.

1. Open Arduino IDE -> File -> Preferences.
2. Add this URL to "Additional boards manager URLs":

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

3. Go to Tools -> Board -> Boards Manager, search `esp32`, and install version `3.0.7`.
4. Select Tools -> Board -> esp32 -> `ESP32S3 Dev Module`.
5. Install the required Arduino libraries, including `LovyanGFX` and `PubSubClient`.

> Image placeholder: insert How-to-use prerequisite screenshots.

### Getting Started

#### Step 1 -- Clone or Download the Code

```bash
git clone https://github.com/Annie-Zhu1210/CASA0021_GPP_Group1.git
```

Open the final sketch in Arduino IDE:

```text
MoodLink/status_timezone_hotspot/status_timezone_hotspot.ino
```

> Image placeholder: insert screenshot showing the code folder / Arduino IDE opening step.

#### Step 2 -- Create `config.h`

Copy `config.example.h` and rename the copy to `config.h`:

```bash
cp MoodLink/status_timezone_hotspot/config.example.h MoodLink/status_timezone_hotspot/config.h
```

Then edit `config.h` with your Wi-Fi hotspot name, pairing code, MQTT broker settings, and device identity.

For one device, use:

```cpp
#define DEVICE_ID 1
```

For the other paired device, use:

```cpp
#define DEVICE_ID 2
```

> Image placeholder: insert Figure showing `config.example.h`.

#### Step 3 -- Flash the Device

Connect the ESP32-S3 board by USB, select the correct board and port in Arduino IDE, then upload the sketch. Repeat the process for the second device, making sure the two devices use different `DEVICE_ID` values.

> Image placeholder: insert flashing / upload screenshots.

#### Step 4 -- Using MoodLink

On startup, MoodLink shows a welcome screen and then a network check page. If Wi-Fi has not been configured, choose Wi-Fi setup. The device will create a temporary hotspot. Connect a phone to the hotspot and use the setup page to select or manually enter the home Wi-Fi network, password, and pairing code.

After both devices are online, rotate the knob to change your emotional status. Long press the knob to open the settings menu, where users can configure date and time, Wi-Fi, world clocks, and schedules.

> Image placeholder: insert Figure showing System Logic and User Journey.

## Production Costs

The prototype hardware components cost approximately £66 per pair. Filament consumption is approximately 820g per pair based on slicer output. Enclosure printing was carried out using university facilities, while commercial printing services would cost more due to labour. At production scale, labour costs were estimated using Formula 1. Unit costs can be significantly reduced through bulk purchasing of components, and the ESP32-S3-WROOM-1 development board can be replaced by a bare ESP32-S3 chip integrated on a PCB, eliminating development board costs. At the £175 Kickstarter price, our prototype has a 46.48% gross margin, while production at scale has an approximate 60% gross margin.

> Image placeholder: insert Formula 1 showing total labour cost per two MoodLink devices.

> Image placeholder: insert Figure showing production vs. at-scale cost breakdown.

## Sustainability

The ESP32 module has an expected lifespan of over 10 years under friendly environmental conditions. The modular design of MoodLink allows individual components to be replaced if anything fails rather than discarding the entire device. This supports the lifespan and repairability of MoodLink. The ESP32 spends most of its time waiting for MQTT messages and updating periodically, without continuous heavy computation, allowing modest energy consumption. In addition, the bio-based PLA filament of the 3D-printed enclosure supports environmental sustainability.

## Evaluation and Future Work

Overall, the product works well and fulfils its purpose. The main hardware limitation was the TFT screen. The ILI9488 display sometimes produced dirty pixels, colour stripes, and unclear text when emojis or menus refreshed. We reduced this by changing the LovyanGFX display settings, but the problem was not completely removed. This suggests that the current screen model, 8-bit parallel wiring, and driver configuration are not ideal for a final product. In the future, we would switch to a display with better-documented ESP32-S3 support, such as an SPI-based ILI9341 or ST7796 screen, or an integrated ESP32-S3 display board with tested examples (lovyan03, n.d.; Ilitek, 2011; Sitronix Technology, 2014).

The second improvement is power. Our current prototype must remain USB-powered, which limits where it can be used. A future version should include a rechargeable LiPo battery, charging circuit, and power management, similar to ESP32 boards with built-in battery charging and monitoring (Espressif Systems, n.d.-a; Microchip Technology, 2020).

Finally, the magnetic accessory system could be expanded. At the moment, personalised accessories can be attached physically. In future, NFC tags inside accessories could allow the device to recognise attached accessories and automatically change the system theme to match, using standard NFC modules such as the PN532 (NXP Semiconductors, 2017).

## Conclusion

MoodLink is a pair of long-distance communication devices that addresses the challenge of staying emotionally present with someone far away across different time zones. It uses MQTT messaging, Wi-Fi provisioning, automatic time synchronisation, and a custom TFT display interface, housed in a curved, rounded enclosure that helps long-distance relationships feel more mentally connected. From a basic status and time display to a polished interface with automatic Wi-Fi scanning, IP-based time detection, and online/offline presence awareness, and from a sharp, rigid-edged design to a softened rounded enclosure, the evolution of MoodLink improves both reliable real-time emotional status sharing and the overall sense of companionship.

## The Team

Below are the contact details for the MoodLink team. Please contact the relevant team member for any queries on the product or its reproduction.

| Name | Email | Main Responsibilities |
| --- | --- | --- |
| Annie Zhu | zcakxz4@ucl.ac.uk | Initial Wi-Fi setup, MQTT data communication, and main screen layout |
| Yussr Osman | yussr.osman.25@ucl.ac.uk | Hardware wiring and graphics design |
| Ziyi Wang | zczq912@ucl.ac.uk | Enclosure modelling |
| Yewei Bian | ucfnybi@ucl.ac.uk | Time and timezone settings, auto time sync, schedule function, Wi-Fi setup UX improvements, MQTT presence detection, and UI refresh optimisation |

## AI Acknowledgement

This project acknowledges the use of AI in an assistive role in line with UCL guidelines, including support for coding, proofreading, grammar, and document structure.

## References

Apple Inc. (2024) *Use iMessage apps on iPhone*. Available at: https://support.apple.com/en-gb/guide/iphone/iph12d2cee48/ios (Accessed: 17 April 2026).

Bandyopadhyay, S. and Bhattacharyya, A. (2013) 'Lightweight Internet protocols for web enablement of sensors using constrained gateway devices', *Proceedings of the International Conference on Computing, Networking and Communications (ICNC)*. doi: https://doi.org/10.1109/ICCNC.2013.6504105.

Cacioppo, J.T. and Patrick, W. (2008) *Loneliness: Human Nature and the Need for Social Connection*. New York: W.W. Norton & Company.

Cartoon Network (2015) *We Bare Bears* [Television series]. Created by Daniel Chong. Atlanta, GA: Turner Broadcasting System.

Davis, K., Owusu, E., Marcenaro, L., Feijs, L., Regazzoni, C. and Hu, J. (2017) 'Effects of ambient lighting displays on peripheral activity awareness', *IEEE Access*, 5, pp. 9318-9335.

Dey, A.K. and de Guzman, E. (2006) 'From awareness to connectedness: the design and deployment of presence displays', in *Proceedings of the SIGCHI Conference on Human Factors in Computing Systems (CHI '06)*. New York: ACM Press, pp. 899-908. doi: https://doi.org/10.1145/1124772.1124905.

DiGiovanni, A.M., Valshtein, T.J., Harris, A., Zoppolat, G., Balzarini, R. and Slatcher, R.B. (2026) 'Investigating the social benefits of long-distance romantic relationships during the COVID-19 pandemic', *Journal of Social and Personal Relationships*. Available at: https://journals.sagepub.com/doi/10.1177/02654075251371365 (Accessed: 17 April 2026).

Espressif Systems (2024a) *ESP32-S3 Technical Reference Manual*. Available at: https://documentation.espressif.com/esp32-s3_technical_reference_manual_en.pdf (Accessed: 17 April 2026).

Espressif Systems (2024b) *SPIFFS Filesystem - ESP-IDF Programming Guide*. Available at: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/spiffs.html (Accessed: 17 April 2026).

Espressif Systems (n.d.-a) *ESP32-S3 Series Datasheet*. Available at: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf (Accessed: 22 April 2026).

Espressif Systems (n.d.-b) *Preferences*. Available at: https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/preferences.html (Accessed: 22 April 2026).

Espressif Systems (n.d.-c) *System Time*. Available at: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/system_time.html (Accessed: 22 April 2026).

Hassenzahl, M., Heidecker, S., Eckoldt, K., Diefenbach, S. and Hillmann, U. (2012) 'All you need is love: current strategies of mediating intimate relationships through technology', *ACM Transactions on Computer-Human Interaction*, 19(4), Article 30.

Ilitek (2011) *ILI9341: a-Si TFT LCD Single Chip Driver Datasheet*. Available at: https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf (Accessed: 22 April 2026).

Ilitek (2013) *ILI9488 a-Si TFT LCD Single Chip Driver: 320(RGB) x 480 Resolution, 16.7M-colour, Version 1.00*. Available at: https://www.elecrow.com/download/ILI9488%20Data%20Sheet_100.pdf (Accessed: 17 April 2026).

IP-API (n.d.) *JSON API*. Available at: https://ip-api.com/docs/api:json (Accessed: 22 April 2026).

lovyan03 (n.d.) *LovyanGFX*. Available at: https://github.com/lovyan03/LovyanGFX (Accessed: 22 April 2026).

Microchip Technology (2020) *MCP73831/2 Miniature Single-Cell, Fully Integrated Li-Ion, Li-Polymer Charge Management Controllers*. Available at: https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP73831-Family-Data-Sheet-DS20001984H.pdf (Accessed: 22 April 2026).

Naik, N. (2017) 'Choice of effective messaging protocols for IoT systems: MQTT, CoAP, AMQP and HTTP', *2017 IEEE International Systems Engineering Symposium (ISSE)*. doi: https://doi.org/10.1109/SysEng.2017.8088251.

Neustaedter, C. and Greenberg, S. (2012) 'Intimacy in long-distance relationships over video chat', *Proceedings of the SIGCHI Conference on Human Factors in Computing Systems*, Austin, May 2012, pp. 753-762.

NXP Semiconductors (2017) *PN532/C1 Near Field Communication (NFC) Controller*. Available at: https://www.nxp.com/docs/en/nxp/data-sheets/PN532_C1.pdf (Accessed: 22 April 2026).

Sitronix Technology (2014) *ST7796S Datasheet*. Available at: https://www.alldatasheet.com/datasheet-pdf/pdf/1775165/SITRONIX/ST7796S.html (Accessed: 22 April 2026).

Thomas, F. and Johnston, O. (1981) *The Illusion of Life: Disney Animation*. New York: Disney Editions.

Violet (2025) *Nabaztag*. Available at: https://nabaztag.com/ (Accessed: 17 April 2026).
