# MoodLink

## Introduction

Living in an increasingly connected and digitised world has contributed to the growth of long-distance relationships, including couples, families living apart, and international friends (DiGiovanni et al., 2026). Although existing technologies such as phones, social media, and messaging platforms facilitate communication, differences in time zones, work schedules, and daily routines often create uncertainty about when contact is appropriate. This challenge is particularly evident in romantic relationships: Neustaedter and Greenberg (2012) found that long-distance couples often experienced feelings of disconnection because they were unable to easily sense their partner's mood, energy level, and availability, which in turn could produce anxiety not fully resolved through calls alone. More broadly, loneliness and perceived social disconnection from significant others can negatively affect both psychological and physical well-being, especially when separation is prolonged by distance (Cacioppo and Patrick, 2008). At the same time, emotional presence remains an important but often unmet need in long-distance relationships (Dey and de Guzman, 2006). Research on ambient IoT further suggests that such devices can communicate information passively without demanding users' full attention (Davis et al., 2017; Hassenzahl et al., 2012). MoodLink was developed as a Kickstarter project in response to this gap, using ambient IoT technology to help couples, friends, and families maintain awareness of one another's feelings and availability without the fear of interrupting. Rather than requiring direct conversation, it supports passive awareness and lightweight communication, allowing users to feel each other's presence in everyday life. One key scenario is the moment of hesitation before making contact, when a user can simply glance at the paired device to judge whether the other person may be asleep, busy, unavailable, or emotionally low before deciding whether to reach out.

## Technical Overview (Hardware and Software)

### Device System

#### Hardware

<p align="center">
  <img src="/Media/Images/schematics.png" width="70%" alt="This is the schematics" /><br>
  <sub>Figure 1. Schematics</sub>
</p>

<p align="center">
  <img src="/Media/Images/hardware.png" width="70%" alt="This is the hardware components" /><br>
  <sub>Figure 2. Hardware components</sub>
</p>

MoodLink's hardware components are outlined in the table above. Components are soldered on a perfboard to minimise the risk of disconnection, ensuring stable shared power rails and grounds. The components were chosen with the aim of facilitating passive interactions for users that feel tactile and deliberate.

#### Functionalities and Controlling Code

<p align="center">
  <img src="/Media/Images/screen_display/welcome_screen.jpg" width="40%" alt="This is the welcome screen" /><br>
  <sub>Figure 3. Welcome screen</sub>
</p>

MoodLink displays consist of a welcome screen that leads users to the main display, which contains the five statuses. Two additional displays were added to improve user interactions with the device: an offline page and a checking page when a partner's connection is not yet confirmed.

<p align="center">
  <img src="/Media/Images/screen_display/offline_status.jpg" width="40%" alt="This is the offline status" /><br>
  <sub>Figure 4. Offline status screen</sub>
</p>

The choice of the ESP32 board allows a convenient Wi-Fi setup process and keeps the interaction within the device itself without a mobile app. Rather than hardcoding network credentials, we built a captive portal for initial setup, which is commonly used by commercial IoT products. Each MoodLink device connects to the internet via the ESP32-S3's built-in Wi-Fi. On first setup, the device broadcasts a temporary hotspot. When users connect their phone to it, a configuration page is served automatically for inputting the home Wi-Fi password and their pairing code. The active surrounding network scanning feature from ESP32 boards was then included. A scannable list was added so users can select their Wi-Fi. The manual input design is retained as a fallback for networks that do not display in scan results. The shared pairing code is hardcoded into the device, ensuring only the intended pair of devices can communicate. In the end, the credentials are saved in the ESP32's non-volatile storage with a bright Wi-Fi icon on the screen top bar, and devices can automatically reconnect on every reboot without repeating the setup process.

The emoji status display acts as the main communication element of MoodLink. The emojis are all drawn using geometric primitives such as arcs and lines instead of font glyphs and bitmap images (lovyan03, n.d.). This ensures that the emojis scale smoothly without any pixelisation throughout different render contexts. Font glyphs lack the required expressiveness for the emojis to represent mood, and bitmaps are prone to pixelisation when scaled (Ilitek, 2013). The bitmap images would have to be stored on the ESP32 using a SPIFFS system, which would have taken up most of the ESP32 SRAM (Espressif Systems, 2024a; Espressif Systems, 2024b). A global `SCALE` float is multiplied by every measurement in the drawing functions, allowing the same drawing code to render three contexts. A schedule page, self-panel, and partner panel dismiss the need for multiple versions of the drawing functions. The partner panel on the left takes up most of the screen, allowing users to easily see their partner's status, whilst the user's own status is kept smaller. Moreover, the scales for each context are saved and restored, ensuring the size of the function does not affect the layout.

<table align="center">
  <tr>
    <td align="center">
      <img src="/Media/Images/screen_display/Busy_status.jpg" alt="This is the busy status" height="330" /><br>
      <sub>Figure 5. Busy status</sub>
    </td>
    <td align="center">
      <img src="/Media/Images/screen_display/bad_day_status.jpg" alt="This is the bad day status" height="330" /><br>
      <sub>Figure 6. Bad day status</sub>
    </td>
  </tr>
</table>

The five statuses are kept simple but expressive to allow for a clear understanding of the user's mood. The miss you status is inspired by the iOS heart reaction, a familiar visual for users (Apple Inc., 2024). The sleeping status is inspired by the *We Bare Bears* cartoons (Cartoon Network, 2015). To prevent tearing from redrawing the bear graphic at parallel bus speeds every 90ms, all elements apart from the Z animation are drawn once using `startSleepScene()`, with each frame only deleting the previous Z position via bounding boxes in `zPrev[i]`. For the Z animation to work, three requirements must be fulfilled. The partner is online, their status is sleeping, and the bear has been illustrated on the screen. To allow the Zzz to loop naturally, the Z characters have their own offsets varying in colour, size, and position. The animation also uses a non-blocking `millis()` timer. This allows the function to work after 90ms to ensure that the animation does not block button handling, clock updates, or MQTT. To mitigate visual clutter, the user's sleeping status was changed to three Zs, whilst the partner's sleeping status was kept the same.

<table align="center">
  <tr>
    <td align="center">
      <img src="/Media/Images/screen_display/miss_you_status.jpg" alt="This is the miss you status" height="330" /><br>
      <sub>Figure 7. Miss you status</sub>
    </td>
    <td align="center">
      <img src="/Media/Images/screen_display/sleeping _tatus.jpg" alt="This is the sleeping status" height="330" /><br>
      <sub>Figure 8. Sleeping status</sub>
    </td>
  </tr>
</table>

For the time and scheduling function, our design combines local interaction with online services and saved settings on the ESP32-S3 board. In the early version of MoodLink, time and timezone were set manually by the rotary encoder, which was inconvenient after restarts or when used in different countries. So we added an Auto Time mode to improve this. When connected to Wi-Fi, it can obtain the UTC offset through an IP-based API and synchronise the clock with NTP to automatically update the time and time zone (Espressif Systems, n.d.-c; IP-API, n.d.). We also kept the manual option. It remains useful when the network is unavailable or users want direct control. The ESP32 saves the selected mode and timezone in Preferences so that the main settings are retained after restart (Espressif Systems, n.d.-b).

<table align="center">
  <tr>
    <td align="center">
      <img src="/Media/Images/AutoTime.jpg" alt="This is the Auto Time Mode" height="220" /><br>
      <sub>Figure 9. Auto Time mode</sub>
    </td>
    <td align="center">
      <img src="/Media/Images/ManualTime.jpg" alt="This is the Manual Time Mode" height="220" /><br>
      <sub>Figure 10. Manual Time mode</sub>
    </td>
  </tr>
</table>

The schedule function was also developed step by step. Firstly, device status could only be changed manually by the knob. We extended this into a schedule system so that users can plan emotional states in advance. Each schedule includes a target status, start time, end time, and a repeat rule (once, daily, weekly, or monthly). We also added checks so that invalid schedules, such as an end time earlier than the start time, cannot be saved. During use, the device checks active schedules and changes the status automatically; then it returns to the previous status afterwards. This makes time in our project not only for display, but also a way to control the device's behaviour.

<p align="center">
  <img src="/Media/Images/Schedule.png" width="40%" alt="This is the Schedule Function" /><br>
  <sub>Figure 11. Schedule function</sub>
</p>

Issues with pixel corruption and colour display in the display occurred due to the ILI9488 driver used in the TFT screen hardware specification not matching the LovyanGFX configuration. This was addressed through six explicit settings in the code. `cfg.dlen_16bit = false;` was vital to allowing the ILI9488 to support the LovyanGFX default 16-bit colour parallel bus by force correcting the 18-bit transmission. The signal sent by the default 2MHz was unstable in the ESP32 parallel bus, making updates unreliable. Changing it to `cfg.freq_write = 8000000` resolved it. An explicit read frequency, `cfg.freq_read = 4000000`, was needed to reduce unpredictable display colours, as undefined read clocks lead to unpredictable results. Readability was disabled using `readable = false` to prevent bus conflicts, as the display's read pin is tied to a high voltage of 3.3V. ILI9488 and LovyanGFX expect colour data in varying formats (RGB, BGR). `rgb_order = false` was added to ensure the correct channel order. LovyanGFX's assumption of bus sharing was corrected.

<p align="center">
  <img src="/Media/Images/screen_display/pixelized_free_status.jpg" width="40%" alt="This is the pixelated free status display" /><br>
  <sub>Figure 12. Pixelated free status display</sub>
</p>

The two MoodLink devices communicate over MQTT, which is a lightweight communication protocol widely used in IoT products (Bandyopadhyay and Bhattacharyya, 2013). The selection of MQTT allows a direct communication approach without knowing IP addresses and a robust system with retained status messages after device reconnection (Naik, 2017). Each device publishes to four topics: emotional status, time in Unix timestamp, timezone index, and heartbeat. Status updates are transmitted immediately on user interaction. The heartbeat was a final design to improve the online/offline indication. Each device publishes a heartbeat every five seconds regardless of activities. If no message is received from the partner device for 130 seconds, the device marks the paired device as offline and displays the time when the partner was last seen. This design ensures the connection state is always communicated to the user rather than silently failing, which is central to MoodLink's aim of providing emotional reassurance rather than uncertainty.

### Enclosure

The enclosure (Figure 13 - Figure 15) was developed as a communicative part. All models were built in Fusion 360 and 3D printed in PLA. It integrates all the components and internal wiring into a compact desktop object. The screen is recessed into the upper section. The rotary knob is placed centrally to support comfortable interaction. A knob cover (Figure 16) unifies the overall appearance colour and a logo nameplate (Figure 17) highlights the unique product features.

<table align="center">
  <tr>
    <td align="center">
      <img src="/Media/Images/enclosure _1.png" alt="This is the assembled enclosure 3D models" height="330" /><br>
      <sub>Figure 13. Assembled enclosure 3D model</sub>
    </td>
    <td align="center">
      <img src="/Media/Images/enclosure _2.png" alt="This is the front-right upper view of all models" height="330" /><br>
      <sub>Figure 14. Front-right upper view of enclosure</sub>
    </td>
    <td align="center">
      <img src="/Media/Images/enclosure _3.png" alt="This is the lower-left rear view of all models" height="330" /><br>
      <sub>Figure 15. Lower-left rear view of enclosure</sub>
    </td>
  </tr>
</table>

<table align="center">
  <tr>
    <td align="center">
      <img src="/Media/Images/Logo_3DModel.png" alt="This is the Logo nameplate" height="220" /><br>
      <sub>Figure 16. Logo nameplate</sub>
    </td>
    <td align="center">
      <img src="/Media/Images/button.png" alt="This is the button" height="220" /><br>
      <sub>Figure 17. Knob cover</sub>
    </td>
  </tr>
</table>

The design was informed by Nabaztag, an ambient device whose rounded and character-like form showed that connected products can communicate through both presence and function (Violet, 2025). Disney's principle of appeal was referenced in a design sense to guide overall friendliness (Thomas and Johnston, 1981). This informed the decision to treat the screen as a face to let it sit naturally. Figure 18 shows the personalised accessories, currently including Santa hat, rabbit ears, and Sorting Hat, which extended this logic by supporting a sense of company and ownership. The connection of the accessories was achieved through six magnetic points.

<p align="center">
  <img src="/Media/Images/accessories .png" width="70%" alt="These are the personalised accessories (from left to right: Santa hat, Sorting Hat, rabbit ears)" /><br>
  <sub>Figure 18. Personalised accessories</sub>
</p>

For the iteration, an earlier prototype (Figure 19) used a rectangular screen on a stand, which was functional but visually far from a companion device. A later sketch (Figure 20) explored a more rounded enclosure with a clearer head-body relationship. A fully integrated shell shown in Figure 21  was also considered, but a separable structure was adopted because it is easier for debugging and reassembly. These changes improved both the emotional character and the practical usability of MoodLink.

<table align="center">
  <tr>
    <td align="center">
      <img src="/Media/Images/Sketch_first_version.png" alt="This is the Sketch of the first version of the enclosure" height="330" /><br>
      <sub>Figure 19. First enclosure sketch</sub>
    </td>
    <td align="center">
      <img src="/Media/Images/Sketch_second_version.png" alt="This is the Sketch of the second version of the enclosure" height="330" /><br>
      <sub>Figure 20. Second enclosure sketch</sub>
    </td>
    <td align="center">
      <img src="/Media/Images/enclosure _3.png" alt="This is the version with head and body integrated" height="330" /><br>
      <sub>Figure 21. Integrated head-body version</sub>
    </td>
  </tr>
</table>

## How to Use

### Prerequisites

<div align="center">
  <img src="/Media/Images/HowToUse1.png" width="100%" alt="This is the Hardware setup and enclosure" />
  <img src="/Media/Images/HowToUse2.png" width="100%" alt="This is the Arduino IDE setup and ESP32 package" />
</div>

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

<div align="center">
  <img src="/Media/Images/HowToUse3.png" width="100%" alt="This is the required libraries" />
</div>

### Getting Started

#### Step 1 - Clone or Download the Code

```bash
git clone https://github.com/Annie-Zhu1210/CASA0021_GPP_Group1.git
```

Open MoodLink/status_timezone_hotspot/status_timezone_hotspot.ino in Arduino IDE.

#### Step 2 - Create Your `config.h`

<div align="center">
  <img src="/Media/Images/HowToUse4.png" width="100%" alt="This is the step 2" />
</div>

<p align="center">
  <img src="/Media/Images/config.example.h.png" width="70%" alt="This is the config.example.h file" /><br>
  <sub>Figure 22. config.example.h file</sub>
</p>

#### Step 3 - Flash the Device

<div align="center">
  <img src="/Media/Images/HowToUse5.png" width="100%" alt="This is the step 3" />
</div>

#### Step 4 - Using MoodLink

<div align="center">
  <img src="/Media/Images/HowToUse6.png" width="100%" alt="This is the step 4" />
</div>

<p align="center">
  <img src="/Media/Images/System_Logic.png" width="70%" alt="This is the System Logic and User Journey" /><br>
  <sub>Figure 23. System Logic and User Journey</sub>
</p>

## Production Costs

In Figure 24, the prototype hardware components cost approximately £66 per pair. The filament consumption is approximately 820g per pair based on slicer output. Enclosure printing was carried out using university facilities, while commercial printing services will cost more due to labour. At the production scale, labour costs were estimated based on Formula 1. Unit costs can be significantly reduced through bulk purchasing of components, and the ESP32-S3-WROOM-1 board can be replaced by a bare ESP32-S3 chip integrated on a PCB, eliminating development board costs. At the £175 Kickstarter price, our prototype has a 46.48% gross margin, while production at scale has an approximate 60% gross margin.

<p align="center">
  <img src="/Media/Images/Cost_Formula.png" width="100%" alt="This is the Formula 1: Total Labour Cost per 2 MoodLink Devices" /><br>
  <sub>Formula 1. Total Labour Cost per 2 MoodLink Devices</sub>
</p>

<p align="center">
  <img src="/Media/Images/Cost_Breakdown.png" width="100%" alt="This is the Production vs. At Scale Cost Breakdown" /><br>
  <sub>Figure 24. Production vs. At Scale Cost Breakdown</sub>
</p>

## Sustainability

The ESP32 module has an expected lifespan of over 10 years under friendly environmental conditions. The modular design of MoodLink allows individual components to be replaced if anything fails rather than discarding the entire device. These guarantee the lifespan and the repairability of MoodLink. The ESP32 spends most of its time waiting for MQTT messages and updating periodically, without continuous heavy computation, allowing a modest energy consumption. Besides, the bio-based PLA filament of the 3D-printed enclosure enhances environmental sustainability.

## Evaluation and Future Work

Overall, the product works well and fulfils its purpose. The main hardware limitation was the TFT screen. The ILI9488 display sometimes produced dirty pixels, colour stripes, and unclear text when emojis or menus refreshed. We reduced this by changing the LovyanGFX display settings, but the problem was not completely removed. This suggests that the current screen model, 8-bit parallel wiring, and driver configuration are not ideal for a final product. In the future, we would switch to a display with better-documented ESP32-S3 support, such as an SPI-based ILI9341 or ST7796 screen, or an integrated ESP32-S3 display board with tested examples (lovyan03, n.d.; Ilitek, 2011; Sitronix Technology, 2014).

The second improvement is power. Our current prototype must remain USB-powered, which limits where it can be used. A future version should include a rechargeable LiPo battery, charging circuit, and power management, similar to ESP32 boards with built-in battery charging and monitoring (Espressif Systems, n.d.-a; Microchip Technology, 2020).

Finally, the magnetic accessory system could be expanded. At the moment, personalised accessories can be attached physically. In future, NFC tags inside accessories could allow the device to recognise attached accessories and automatically change the system theme to match, using standard NFC modules such as the PN532 (NXP Semiconductors, 2017).

## Conclusion

MoodLink is a pair of long-distance communication devices that solves the challenge of staying emotionally present with someone far away across different time zones. It uses MQTT messaging, Wi-Fi provisioning, automatic time synchronisation, and a custom TFT display interface, housed in a curved, rounded enclosure that keeps long-distance relationships mentally connected. From a bare status and time display to a polished interface with automatic Wi-Fi scanning, IP-based time detection, and online/offline presence awareness, and from a sharp and rigid-edged design to a softened, rounded enclosure, the evolution ensures not only reliable real-time emotional status sharing but also the seamless user experience and a sense of companionship.

## The Team

Below are the contact details for the MoodLink Team. Please contact the relevant team member for any queries on the product or its reproduction.

| Name | Email | Main Responsibilities |
| --- | --- | --- |
| Annie Zhu | zcakxz4@ucl.ac.uk | Initial Wi-Fi Setup, MQTT Data Communication, and Main Screen Layout |
| Yussr Osman | yussr.osman.25@ucl.ac.uk | Hardware wiring and graphics designer |
| Ziyi Wang | zczq912@ucl.ac.uk | Enclosure modeling |
| Yewei Bian | ucfnybi@ucl.ac.uk | Time and Timezone Settings, Auto Time Sync, Schedule Function, Wi-Fi Setup UX Improvements, MQTT Presence Detection, and UI Refresh Optimisation |

## AI Acknowledgement

This project acknowledges the use of AI in an assistive role in line with UCL guidelines to assist with coding and proofreading to help with grammar and structure.

## References

Apple Inc. (2024) *iMessage*. Available at: https://support.apple.com/en-gb/guide/iphone/iph12d2cee48/ios (Accessed: 17 April 2026).

Bandyopadhyay, S. and Bhattacharyya, A. (2013) 'Lightweight Internet protocols for web enablement of sensors using constrained gateway devices', *IEEE Xplore*. doi: https://doi.org/10.1109/ICCNC.2013.6504105.

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

Naik, N. (2017) 'Choice of effective messaging protocols for IoT systems: MQTT, CoAP, AMQP and HTTP', *IEEE Xplore*. doi: https://doi.org/10.1109/SysEng.2017.8088251.

Neustaedter, C. and Greenberg, S. (2012) 'Intimacy in long-distance relationships over video chat', *Proceedings of the SIGCHI Conference on Human Factors in Computing Systems*, Austin, May 2012, pp. 753-762.

NXP Semiconductors (2017) *PN532/C1 Near Field Communication (NFC) Controller*. Available at: https://www.nxp.com/docs/en/nxp/data-sheets/PN532_C1.pdf (Accessed: 22 April 2026).

Sitronix Technology (2014) *ST7796S Datasheet*. Available at: https://www.alldatasheet.com/datasheet-pdf/pdf/1775165/SITRONIX/ST7796S.html (Accessed: 22 April 2026).

Thomas, F. and Johnston, O. (1981) *The Illusion of Life: Disney Animation*. New York: Disney Editions.

Violet (2025) *Nabaztag*. Available at: https://nabaztag.com/ (Accessed: 17 April 2026).
