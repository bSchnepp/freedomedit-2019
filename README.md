# FreedomEdit
A rewrite of FreedomWriter, a text editor I wrote 4 years ago.

## Why this over gedit?
FreedomEdit will eventually be used as my text editor for developing
Feral Waypoint, largely because I've started to find gedit annoying with the
way certain things work on my triple-monitor setup, so I have to rely
on the side bar to open files which is sometimes okay, but sometimes isn't,
and then as well with the limited ways syntax highlighting works and all, such
that I can't apply highlighting to arbitrary text on an arbitrary set of files.
(ie, highlight "UINT64" on anything that includes stdtypes.h, but not on
any other files.)

Likewise, I couldn't just use another text editor where the features don't map
one-to-one, because then I have to get used to new macros and key bindings for
how things work.

Plus, I can embed some more IDE-like features in, while being careful not
to bloat the editor too much. I can add in a generic profile system to
automatically build trivial projects in certain languages (immensely helpful
for reading a Gist and figuring out what's going on), quickly test if something
will cross-compile to another architecture easilly, and incorporate a whole
new build system and all.

## Why not GtkSourceView?
I'm trying to minimize external dependencies as much as possible.
GTK on it's own is a *massive* library. I want FreedomEdit to be as
simple to port to another platform as possible, and GTK is already
hard. Ensuring some module not really part of GTK works too is
putting lots and lots of work in when I just want something that works.

## Why GTK at all?
I need to port GTK *anyway* to bring over Chromium/Iceweasel and whatnot.
This is going to involve a lot of porting, maybe a compatibility
layer and faking Xlib/xcb. Being already cross-platform, hopefully
it's pretty trivial to do so.

Qt is a very, very large library with even more dependencies. I don't want
to use it if I don't have to. It's helpful for cross-compiling, but I want to
keep using C if I can.

Directly drawing using X is silly. It's very hard to port that without
also bringing the entirety of X.
