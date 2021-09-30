# frf

a very loosely forth-inspired interpreted language.



## test programs

these are located in `tests/` for hopefully obvious reasons.

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
