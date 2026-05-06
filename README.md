# UAV Propeller Damage Detection System

A low-cost (~$90) sensor fusion system for pre-flight propeller damage detection in unmanned aerial vehicles. Built around an Arduino, MPU6050 accelerometer, analog microphone, and IR optical RPM sensor — capable of distinguishing healthy vs. damaged propellers within 30–40 seconds of bench testing.

---

## Motivation

Roughly 30% of UAV crashes are propeller-related. Existing pre-flight inspection is visual and subjective; commercial vibration analysis platforms exceed $1,000. This project asks: **can a $90 sensor stack reliably classify common propeller damage modes before takeoff?**

## Hardware

| Component | Purpose | Pin |
|---|---|---|
| Arduino Uno/Nano | Main controller | — |
| MPU6050 (I2C) | Vibration sensing (±2g, currently saturating) | SDA/SCL |
| Analog microphone module | Acoustic energy (peak-to-peak) | A0 |
| IR optical sensor | RPM via interrupt counting | D2 |
| ESC + brushless motor + propeller | Test specimen | D9 (PWM) |

Total bill of materials: ~$90.

## Test Protocol

The Arduino sketch (`propeller_damage_detection.ino`) runs an automated 5-step PWM ramp:

1. **Trigger:** rotate propeller manually >10 IR pulses to start
2. **Ramp:** PWM steps `[20, 25, 30, 35, 40]`, 5 seconds per step
3. **Sampling:** every 100 ms, log RPM, peak-to-peak microphone amplitude, and total horizontal vibration magnitude (√(accX² + accY²))
4. **Cooldown:** 10-second pause between runs
5. **Output:** Serial CSV — `Saat, Test_Asamasi, PWM, RPM, Ses_Enerjisi, Titresim_G, Durum`

Five propeller conditions tested, each with 3 repetitions: **intact**, **notched**, **broken tip**, **cracked**, **mass-imbalanced**.

## Key Findings

- **Notched propeller:** mass loss negligible, but aerodynamic disturbance saturated the ±2g accelerometer at ~2.83G — high sensitivity even for small damage.
- **Broken-tip propeller:** the IR sensor produced a "half-rotation anomaly" because the missing tip can no longer interrupt the optical beam — a 100% reliable damage signature in the optical channel.
- **Decision logic:** combining `sat_ratio` (fraction of saturated accelerometer samples) and `rpm_ratio` (deviation from expected PWM-RPM curve) gives a field-deployable GO/NO-GO decision in 30–40 seconds.

## Known Limitations (Active Development)

1. **Accelerometer saturation.** ±2g is hit on the most dangerous damage modes — switching to ±8g mode is needed to *quantify* damage severity, not just detect presence.
2. **Time-domain only.** Each damage type should produce a distinctive frequency-domain signature (notched → resonance peak, imbalance → 1× rev frequency dominant). FFT-based feature extraction is the next analytical step.
3. **Microphone AGC.** The current module's automatic gain control compresses amplitudes. A constant-gain capture + spectral analysis would unlock acoustics as a third independent verification channel.
4. **Bench-only validation.** All testing was on a fixed rig. Real-world UAV body vibration, prop-wash, and wind add noise — in-flight validation is the credibility step.

## Files

- `propeller_damage_detection.ino` — Arduino firmware (current bench-test version)

## License

MIT — see `LICENSE`.
