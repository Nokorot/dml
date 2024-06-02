# dml (DMenu List format)

This program is an essential part of my day-to-day use of my i3 environment.

It is a parser for a simple key-value data format, intended to list the keys 
with dmenu (or an alternative) and thus selecting a value.

The original idea was to replace [dmenu-run](https://manpages.debian.org/stretch/suckless-tools/dmenu_run.1.en.html), as I wanted the list only to include 
programs that actually are windowd and I use frequently, I decided to read the list 
of programs from a file. However, in order to make it a bit more versatile,
the key may refer to any shell command. e.g. I have `shutdown` meaning to `shutdown now`

The simplest use of the format is very simple. The first word of each line is the key and 
the rest of the line is the command. e.g.

```dml
shutdown       prompt "Shutdown computer?" && shutdown now
exit           prompt "Exit i3?" && i3-msg exit

# This is a comment
#  Programs:
steam          steam
browser        $BROWSER     # You may use environment variables
terminal       $TERMIANL

# Websites:
discord        $BROWSER --app="https://www.discord.com/channels/@me"
```

But why parse this with C++? Initially, I simply used bash, but at some point, 
I wanted to have the ability to separate the list into multiple files (Why? How knows.)
I decided this was a complex enough task, to implement the parser in an actual 
programming language.

```dml
#include ~/.dml/programs.dml
#include ~/.dml/websites.dml
```

The format also supports conditionals and `\` at the end of the line, allowing for 
multi-line commands:

```dml
%if pgrep -x i3 > /dev/null
exit           prompt "Exit i3?" && i3-msg exit
#endif


netflix         i3-msg "workspace $WSmovie"; \
                $BROWSER --app="https://www.netflix.com" & \
                i3-on_open "border pixel 0" "border radius 0" &
```

## Usage and installation

To use the parser with dmenu and bash run 
```console
$ _dml_composer --browse-prg 'dmenu -l 20' | bash
```
or you can use the script `dml` that you can find in the `bin` folder.

To install, run
```console
$ ./build.sh install 
```
This installs both `_dml_composer` and the `dml` script.

