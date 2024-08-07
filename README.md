# Stepper controller

Interface:
- Gepin Control and Status Interface
- via UART

Control:
- target positions
- max velocity
- microstepping resolution

Status:
- current positions
- motor status

Uses the [AccelStepper](https://github.com/waspinator/AccelStepper) library for motion profile and step/dir generation.
The [ioda control software](https://github.com/plex1/ioda_control_sw) was used for testing.
