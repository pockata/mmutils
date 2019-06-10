# mmutils

mmutils (multi-monitor utilities) is a set of utilities for querying xrandr monitor information.

Their main purpose is to be used alongside [wmutils](https://github.com/wmutils/), but they will probably work with anything that's running on X.

## Utilities

Here's a list of what is currently implemented:

* lsm - get a list of monitors (active & connected)
* pfm - get xrandr id of the focused monitor (the monitor which holds the pointer)
* mattr - get information about a monitor (width, height, x/y offset)

## Usage

Because mmutils is intended to work with (inspired by, and even borrowed some code from) wmutils, they have a similar way of operation.

### lsm
By default `lsm` lists all active monitors

```
$ lsm
VGA1
VGA-1-2
HDMI-1-4
```

Passing `-a` will list all active monitors as well as all connected monitors (monitors that are physically connected to a graphics card, but don't have an active X desktop assigned).

```
$ lsm -a
LVDS1
VGA1
VGA-1-2
HDMI-1-4
```

### pfm

Gets the xrandr id of the focused monitor.  
The focused monitor is determined by the pointer coordinates. Whichever monitor holds the pointer is considered as "focused".
```
$ pfm
VGA-1-2
```

This is useful when combined with `mattr`
```
$ mattr whxy $(pfm)
1920 1080 0 0
```

### mattr

Gets information (connected, active, width, height, x/y offset) about a monitor.  
A monitor can be specified by it's xrandr id (LVDS1, VGA-1-2, etc.) or by a window id (e.g. 0x00600096).

**By xrandr id**  

Check if VGA1 is connected and has an X screen attached
```
$ mattr "VGA1" && echo "VGA1 active"
```

Check if VGA1 is connected
```
$ mattr -c "VGA1" && echo "VGA1 connected"
```

Get width and height of VGA1
```
$ mattr wh "VGA1"
1920 1080
```

Get x and y offset of VGA1 
```
$ mattr xy "VGA1"
0 0
```

Useful for piping
```
$ mattr i "VGA1"
VGA1
```

All together now
```
$ mattr ixywh "VGA-1-2"
VGA-1-2 3840 0 1680 1050
```

**By window id**  

You can pass a window id (e.g. `0x00600096`) to `mattr` and you'll get info about the monitor which the window belongs to.  
`mattr` determines if a window belongs to a monitor by comparing the intersection areas between the window and active monitors, so if a window is placed between two monitors, the monitor which displays the largest part of the window is selected.

```
$ mattr iwh $(pfw)
HDMI-1-4 1920 1080
```
Note: `pfw` is part of wmutils. It returns the id of the currently focused window.


### Using them together
Getting info about all active monitors is as simple as
```
$ lsm | xargs mattr iwhxy
VGA1 1920 1080 0 0
VGA-1-2 1680 1050 3840 0
HDMI-1-4 1920 1080 1920 0
```

**Piping**

There's a special case when using `mattr` in conjunction with other command
line tools. Since `mattr` allows both `mid` and `wid` to be passed as an
argument and `i` and `n` returning only `mid`, chaining the output with other
applications can be a bit troublesome, e.g. to get a list of all the windows
and monitors they're shown in you'd need to use something like this:
```
$ mattr i $(lsw)
HDMI2
VGA1
HDMI2
```

This doesn't do us much good as we lose the `wid`s returned by `lsw`. To get
the proper output we need something like this:

```
$ for wid in $(lsw); do
    printf '%s\n' "$(mattr i $wid) $wid"

DisplayPort-2 0x01e0000a
DisplayPort-1 0x00c000f1
DisplayPort-1 0x00c000a6
HDMI-A-2 0x01400003
```

This isn't pretty to look at or write so the `s` parameter comes into play. It
returns the passed argument to `mattr` and streamlines the whole process, e.g.

```
$ mattr si $(lsw)
0x01400010 HDMI2
0x01400010 VGA1
0x01400010 HDMI2
```

Note that `s` is only useful when you're passing a `wid` to `mattr`. In the
case when you're passing a `mid`, `s` is equivalent to `i` and `n`.

## Usage with wmutils

A simple usage example with wmutils would be to change the `contrib` repo script [fullscreen.sh](https://github.com/wmutils/contrib/blob/master/fullscreen.sh) for basic multi-monitor capabilities.  

Currently the window expands to the whole root X window (across all your monitors), which is not what you would want (most of the time).  
We want the expanded window to occupy the full size of it's monitor.

The change is really simple and boils down to replacing the line

```
wtp $(wattr xywh `lsw -r`) $1
```

with (note **m**attr instead of **w**attr)

```
wtp $(mattr xywh $(pfw)) $1
```

Note that this change only illustrates how easily one can modify his current scripts to play nice with multiple monitors. Taking [fullscreen.sh](https://github.com/wmutils/contrib/blob/master/fullscreen.sh)'s current implementation, only one fullscreen window is allowed at any given time. If you want multiple fullscreen windows (per monitor), additional changes should be made.

## Dependencies

mmutils only relies on the XCB library.

## Build & Install

System-wide installation (default PREFIX is `/usr`):

    $ make
    # make install

Here are the variables you can override, along with their default values:

    CC         =  cc
    LD         =  $(CC)
    CFLAGS     =  -std=c99 -pedantic -Wall -Os
    LDFLAGS    =  -lxcb
    PREFIX     =  /usr
    DESTDIR    =

You can override them by passing them on the command line:

    make DESTDIR=/newroot install

Or by setting them via the environment:

    CC=c99; export CC
    make -e

## LICENSE
ISC. See LICENSE file

## TODO
- [x] Manpages
- [ ] Code cleanup
- [ ] Get primary monitor (if selected)

