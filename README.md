# frf

a very loosely forth-inspired interpreted language.

written in purebasic, for reasons.

i didn't really 'do' version control last time I worked on this project so there are two archival branches, which will remain as-is for historical purposes:
*  frf-prototype-1
*  frf-partial-rework

the partial rework was undirected and very much a victim of second system syndrome, and never actually worked.

since frf-prototype-1 was functioning (if kinda simplistic), i intend to try to build off of that and make incremental changes hereafter (although if possible I will draw from the rework where reasonable)

wip builds are in the `build/` folder. i'm currently building windows and linux. mac will come once i get a mac running again.

i have no idea on if the linux builds will work on arbitrary linux systems. gtk2 is likely to be required, beyond that? it is a mystery

eventually i'll start posting releases properly, but as i see it not much point in that until i have a proper environment

## test programs

`factor.frf` is a simple math thingy to get all the prime factors of a provided value.

`ipctest.frf` is an attempt at validating the function of inter-process communication in frf's internal process model.

`replace.frf` was something that i slapped together to edit some ebooks years ago that did not load in my then-preferred ebook reading software. it's honestly not worth much for looking at, although it does demonstrate string manipulation and file handling.

## documentation

in the archival branches, documentation is clearly some kind of attempt at graphing the design as of 2012, i have no idea on how accurate it is at this point.

i am dubious as to their value, so I have removed those files from master.

### `documentation/mufman.txt`
i do now however have a copy of the muf manual from protomuck (with license alongside, since that is a) not my work, and b) i am making no claim that it is my work)

since muf is one of the primary inspirations, it can be seen as a temporary loose design document, although obviously large swaths of it are inapplicable due to the fact this isn't exactly a text-based multiplayer game server w/all of the database particulars that go alongside.

## temp
temp in the archival branches contains mysterious things. i cannot remember their purpose. (except the propertygrid bits, those are probably source examples that I used for `propgrid.pb`)
