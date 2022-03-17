# frf

a very loosely forth-inspired interpreted language.

aimed at cross-platform, with an ultimate eventual eye towards attempting to make games. yes, there are many existing options. no, they aren't suitable for my needs. a casual examination of the tree will probably reveal to the discerning programmer that i don't think like normal people.

work is very ongoing and it is not a stable platform as of yet, although i do make an earnest attempt to guarantee that any given commit will successfully build on at least linux.

it has been tested on linux (xubuntu 20.04-3) and windows 10 (using msys2).

## building


### requirements

1.  bdw-gc (because i am not up for hand-rolling memory management)
2.  libuv (because i'm an event driven girl in an event driven world)

on linux, should just be a git clone and building via `cmake` and then `make`. the `cm.sh` script and `frf` symlinks help keep things clean for me, but are not required. i'm not your mom, build it how you want. you very likely already have bdw-gc, and libuv (and bdw-gc if your distro is lacking it by default) should be installed through whatever package manager your distribution uses.

on windows, you'll need msys2, update it fully, and install packages for `git`, `cmake`, and `mingw-w64-x86_64-gc` via pacman. from there, it should just be the same steps as linux, just in the mingw-64 shell. don't try it in 32-bit; i as yet make no attempts to support 32-bit systems. (that might change if i get around to android support since so many arm things expect apps to be 32-bit).

macos is as yet untested, but i would be shocked if the current build doesn't work just as per linux since i'm sticking so close to plain c.

## test programs

these are located in `tests/` for hopefully obvious reasons.

`prototype.frf` is my current scratch space for testing features as i add them to the implementation.

`infloop.frf` is an infinite loop. 🤷‍♀️

`factor.frf` is a simple math thingy to get all the prime factors of a provided value.

`ipctest.frf` is an attempt at validating the function of inter-process communication in frf's internal process model.

`replace.frf` was something that i slapped together to edit some ebooks years ago that did not load in my then-preferred ebook reading software. it's honestly not worth much for looking at, although it does demonstrate string manipulation and file handling.

## historical notes
frf was originally written in purebasic, for reasons.

i didn't really 'do' version control the first times I worked on this project so there are two archival branches, which will remain as-is for historical purposes:
*  `frf-prototype-1`
*  `frf-partial-rework`

the partial rework was undirected and very much a victim of second system syndrome, and never actually worked.

since frf-prototype-1 was functioning (if kinda simplistic), i intended to try to build off of that and make incremental changes, the results of which can be found in `frf-failed-revival`. i got it working with a newer purebasic version, but i quickly ran into some of the demoralization issues that caused me to halt before, mostly having to do with the unfortunate state of the purebasic language and its community.
## documentation

in the archival branches, documentation is clearly some kind of attempt at graphing the design as of 2012, i have no idea on how accurate it is at this point.

i am dubious as to their value, so I have removed those files from master.

### `documentation/mufman.txt`
i do now however have a copy of the muf manual from protomuck (with license alongside, since that is a) not my work, and b) i am making no claim that it is my work)

since muf is one of the primary inspirations, it can be seen as a temporary loose design document, although obviously large swaths of it are inapplicable due to the fact this isn't exactly a text-based multiplayer game server w/all of the database particulars that go alongside.

## temp
temp in the archival branches contains mysterious things. i cannot remember their purpose. (except the propertygrid bits, those are probably source examples that I used for `propgrid.pb`)
