# simple_clock

## Description
This repository contains the firmware for a replacement clock that I put into an old chassis.  The old clock was an analog clock.  I rebuilt a new one as a digital clock using 7-segment LEDs.  It uses the Blue Pill board ([Link](https://stm32-base.org/boards/STM32F103C8T6-Blue-Pill)) as its processor and relies on the RTC module of the STM32.  I removed the battery and replaced with a DC jack.  I also added a backup battery for the RTC.

The replacement clock must fit in the old chassis, so I made a bracket with 3D printing.  The mainboard that holds the Blue Pill and the 7-segments was designed in Eagle, which the schematics and gerber files are included in this repo.

For the firmware, the project was created using STM32CubeMX which uses STM32 HAL as the library.  The code is actually simple but I made it look harder than it actually is.  The big chunk of code is how I handle buttons.  The part about time keeping is not so much as the RTC on this MCU is very easy to use.  Nonetheless, there are a number of problems with the HAL library.  I'm listing the ones that affected this project here.
- The RTC's `HAL_RTC_GetTime` must be followed by the `HAL_RTC_GetDate` in order for it to work properly.
- The initialization routine `MX_RTC_Init`, by default, intializes the RTC's time to 0.  This part must be removed manually in order to use backup time from RTC.
- The default IRQ handler for Systick Interrupt is not called in the interrupt routine.  You must add it manually.  This part is a minor problem and not relevant to this project but I use it so I decided to mention here anyway.

## Notes on Board Assembling
- Make sure to take care of vias first if you develop the board yourself.
- Then do the MAX7219 next as it needs to be soldered on both sides.
- The rest of components can be soldered anytime afterwards.
