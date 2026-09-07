Google Calendar Clock

A smart clock built with an ESP32 that visualizes Google Calendar events using an LED ring.

<table>
  <tr>
    <td align="center">
      <img width="500" alt="image" src="https://github.com/user-attachments/assets/736f689e-a1e3-4a83-aea3-71b61e625eef" />
    </td>
    <td align="center">
      <img width="500" alt="Screenshot_20260907_132023" src="https://github.com/user-attachments/assets/f90bf5ae-d233-4f2a-9c86-68226727a709" />
    </td>
  </tr>
</table>


Features
- Google Calendar sync via a Google Apps Script web endpoint
- LED animations for visualizing calendar events
- Buzzer notifications with configurable tone presets
- Touch control - the metal body of the clock acts as one large button
- Wi-Fi connectivity
- Automatic time synchronization
How it works

The ESP32 connects to Wi-Fi and periodically fetches calendar events from a Google Apps Script that exposes a URL endpoint.

Calendar events are displayed on the LED ring around the clock. Active events can trigger animations and buzzer notifications, while touching any part of the metal body can acknowledge the current event.

Hardware
ESP32
WS2812B LED ring
Touch-capable metal clock body
Passive buzzer
