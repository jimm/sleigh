# Sleigh

Sleigh is a Studiologic Sledge program organizer for MacOS. It's similar to
the Sledge Sound Mapper application that Studiologic provides, but I've had
problems getting that one to receive programs from the Sledge.

Sleigh ONLY works on MacOS. (Tech note: I could modify Sleigh to use the
cross-platform PortMidi library, but it has problems with large sysex
dumps.)

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

The left window contains programs loaded from a sysex dump file.

The right window contains programs sent from the Sledge and any programs you
copy over from the left window.

Programs are not sent to the Sledge until you use the **t** transmit
command.

Whenever you send a single program or all programs from the Sledge, they are
received by Slegh and displayed in the right window.

# Example Workflows

## Backing Up Your Sledge Programs

Send all programs from the Sledge. As they are received, the right Sledge
window is updated. When they are all received (which will take a few
minutes), you can then use the **s** save command to save a range of
programs to a file.

## Loading Programs Into the Sledge

Load a sysex dump file into the left window. Copy any of those to the right
Sledge window. Move things around if you like. Transmit them to the Sledge.

# Environment Variables

`$SLEIGH_SYSEX_DIR` is the default directory used to load and save sysex
dumps.

# Commands

* **l** - Load a sysex dump file. The loaded file will appear in the
  left window.

* **s** - Save a sysex dump file. The programs that are saved are those in
  the right Sledge window, not the left loaded file window.

* **>** - Copy a range of programs from the left window to the
  right Sledge window.

* **m** - Move a range of programs within the right Sledge window. The
  source slots will be initialized. The destination slots will be
  **overwritten**, but remember that nothing is sent to the Sledge until you
  explicitly send it using the **t** transmit command.

* **t** - Transmit a range of programs to the Sledge.

* **r** - Refresh the screen.

* **q** - Quit.
