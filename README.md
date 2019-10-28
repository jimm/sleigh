# Sleigh

Sleigh is a [Studiologic
Sledge](https://www.studiologic-music.com/products/sledge2/) program
organizer for MacOS. It's similar to the Sledge Sound Mapper application
that Studiologic provides, but I've had problems getting that one to receive
programs from the Sledge. Sleigh is not a fancy GUI program. It runs in your
terminal.

Sleigh ONLY works on MacOS. (Tech note: I could modify Sleigh to use the
cross-platform PortMidi library, but I've had buffer overrun problems with
large sysex dumps.)

One more caveat: Sleigh is run within a terminal.

# Screen Shots

# Installing

There is no easy installer for Sleigh yet. You'll have to download the
source and compile it by running `make`.

# Running

```
$ ./sleigh -l
... list of input and output MIDI endpoints...
$ ./sleigh --input 1 --output 1 --channel 1
```

# Overview

The left window contains programs loaded from a sysex dump file. Load a file
using the **l** load command.

The right window contains programs sent from the Sledge and any programs you
copy over from the left window. Whenever you send a single program or all
programs from the Sledge, they are received by Sleigh and displayed in the
right window.

Programs are not sent to the Sledge until you use the **t** transmit
command.

Save the programs in the Sledge window to a file using the **s** save
command.

You can select single programs or a range of programs in each window using
the mouse. To select or deselect a single program, click it. To select a
range of programs, click on one and shift-click on another. All of the
programs in the range are selected. To deselect a program in the selected
range, shift-click it.

To move programs from the left window to the Sledge window, use the **>**
command. You will be asked for the starting program number. That will be
where the first of the selected programs are copied. The remaining selected
programs will be copied to the Sledge sequentially, with no gaps (even if
there are gaps in the selected programs in the left window.)

You can also copy (**c**) or move (**m**) programs within the Sledge window.
When moving, the old programs are replaced with the standard Sledge "Init"
program.

# Example Workflows

## Backing Up Your Sledge Programs

Send all programs from the Sledge (Press the MIDI button, navigate down,
select "Send All Programs", and click the data entry knob). As each program
is received, the right Sledge window is updated. When they are all received
(which will take a few minutes), you can then use the **s** save command to
save a range of programs to a file.

## Loading Programs Into the Sledge

Load a sysex dump file into the left window. Copy any of those to the right
Sledge window. Move things around if you like. Transmit them to the Sledge.

# Environment Variables

`$SLEIGH_SYSEX_DIR` is the default directory used to load and save sysex
dumps.

# Commands

## Loading and Saving Programs

* **l** - Load a sysex dump file. The loaded file will appear in the
  left window.

* **s** - Save a sysex dump file. The programs that are saved are those in
  the right Sledge window, not the left loaded file window.

## Copying/Moving Programs

* **>** - Copy a range of programs from the left window to the
  right Sledge window.

* **m** - Move a range of programs within the right Sledge window. The
  source slots will be initialized. The destination slots will be
  **overwritten**, but remember that nothing is sent to the Sledge until you
  explicitly send it using the **t** transmit command.

## Sending Programs to the Sledge

* **t** - Transmit a range of programs to the Sledge.

## Miscellaneous Commands

* **r** - Refresh the screen.

* **q** - Quit.

# To Do

- fix active win after file load
- search by name
- filter by category
