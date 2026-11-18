# <img src="html/images/logotst2.gif" alt="HighWire" />

Welcome to the home of the HighWire web browser for the Atari ST, TT, Falcon and FireBee.

HighWire runs on any ST compatible machine, no GDOS required, and renders using SpeedoGDOS, NVDI or fVDI if you have one, but you'll need to install a TCP/IP stack (e.g. STinG, STiK2, MiNTnet) to get online.

If your ST isn't already online, take a look at [MD/Net](https://downloads.neilrackett.com/md-net), which has everything you need to get up and running.

## Installing

- Download the correct release: -68k is pure 68000 without FPU, -020 is for 68020 with FPU to 68060 with FPU (including 68080), -v4e is for ColdFire/FireBee machine.
- Extract the zip file to your hard disk.
- Configure `example.cfg/highwire.cfg` and put it besides `highwire.prg`.
- Run `highwire.prg`.

The internet stack is automatically detected by reading the cookie jar. Detection can be forced by user's settings in the `highwire.cfg`.

## Downloads

- [This fork builds](https://ptonthat.fr/files/highwire/)
- [Official builds](https://downloads.neilrackett.com/atarist-highwire)
- [Snapshots before August 2026](https://atari.joska.no/snapshots/highwire/)

## Building

With [m68k-atari-mint-gcc](http://vincent.riviere.free.fr/soft/m68k-atari-mint/)
and [GEMLib](http://vincent.riviere.free.fr/soft/m68k-atari-mint/archives/gemlib-Git-20230112-bin-cygwin-20230114.tar.bz2) 
and [CFLib](http://vincent.riviere.free.fr/soft/m68k-atari-mint/archives/cflib-Git-20221026-bin-cygwin-20230115.tar.bz2)
and [fdlibm](https://tho-otto.m68k.eu/download/mint/fdlibm-20240425-mint-dev.tar.xz)
installed:

```
make all
```

Other .a and .h libraries are provided in the local lib/ folder.

Modules can be built separatly. For exemple, run from the root directory:

```
make -C modules/decoder.src CPU=68000 img_gif
```

## License

**GPL-3.0-or-later.** See [LICENSE](LICENSE).

HighWire is based, in part, on the Project HighWire source released by Robert
Goldsmith, and has been altered and extended by many contributors since. Code
from before the move to the GPL is also available under its original
zlib-style licence, which is kept, unaltered, in
[docs/highwire.txt](docs/highwire.txt).

The vendored [`lib/`](lib/) libraries keep their own licences: giflib is MIT,
libpng, libjpeg and others have their own permissive licences, and all are compatible.
This software is based in part on the work of the Independent JPEG Group. 
Licenses are grouped in `doc/licenses/` folder's releases.
