# dml (DMenu List format)
This program is an essential part of my day-to-day use of my i3 environment.

It is a parser for a __simple__ key-value data format, intended to list the keys 
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


# Further use
## Vim dml
Clearly, this format may be used for more than only shell commands. 
For example, in Vim I have bound M+d, to open a dml-demnu-window, with a
list of keys mapped to vim-commands.

I have a file `.vim.dml`
```
open_vimrc  tabnew $HOME/.vimrc
open_vdml   tabnew $HOME/.vim.dml
```
My script also sets the env-variable `FT` to be the filetype
so that I may have filetype-specific options
```
%if [ "$FT" = "sh" ]
chmod_exec           !chmod +x %
%endif
```

## sxiv dml
I have also mapped M+d to open a dml-demnu-window from [sxiv](https://github.com/Nokorot/sxiv)
I decided to do this when I wanted the ability to AI upscale directly from sxiv. 
It is also nice, that I don't need to remember all the keybindings for 
rotating and flipping the image (in file).

```
setbg           setbg "$img"
rotate+         convert -rotate 90 "$img" "$img"
rotate-         convert -rotate -90 "$img" "$img"
flip            convert -flop "$img" "$img"
delete          gio trash "$img" && notify-send "$img deleted."
yank_path       echo -n "$img" | xclip -selection clipboard
gimp            gimp "$img"
upscale         prompt "Note that this will upload the image. Proceed?" && upscale "$img"
copy-to         echo '' | dmenu -o 'Path: ' | read p; cp "$img" "$p" 
dragon          dragon "$img"

open_dml_file   $TERMIANL nvim "$HOME/.config/sxiv/main.dml" 
```

I also have the same option (prefixed with `m_`), applied to all marked images
```
%if [ -n "$marked_imgs" ]
m_dragon          cat "$marked_imgs" | xargs dragon
%endif
```

