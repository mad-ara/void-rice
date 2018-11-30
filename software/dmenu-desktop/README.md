# k5-dmenu-desktop
### Fast as the mind application launcher

In my opinion, a menu should be as fast as possible, since when it opens you
already have in mind which application you will want to run. A menu is just an
obstacle between you and the realization of your desire.

That's why I wrote this program (and also because I'm a crazy perfectionist who
loves Unix and C).

It searches for installed applications (which follow the
[freedesktop.org standard](https://standards.freedesktop.org/desktop-entry-spec/latest/)),
sends their names on stdin to an executable (usually [dmenu](http://tools.suckless.org/dmenu/)),
and reads the user's choice from its stdout.

It should work on all systems with a C99 compiler and a POSIX/freedesktop conforming
environment. Dmenu or an equivalent program is also required.

Specifically, I thought it as an alternative to the painfully slow menu of
[i3wm](https://i3wm.org/) (which is written in perl), and the faster
[j4-dmenu-desktop](https://github.com/enkore/j4-dmenu-desktop/), which works pretty
well but is written in c++ (too complex for my taste).

The program is currently an alpha, so it needs various tests and improvements.
Patches and bug reports are welcome.

## News & updates

 * [Telegram](https://telegram.me/matteoalessiocarrara) 
 * [Facebook](https://www.facebook.com/matteoalessiocarrara)
 * [GitHub](https://github.com/matteoalessiocarrara)

