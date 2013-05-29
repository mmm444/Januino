Januino
=======

*Simple monophonic synth for Standuino*

Slighlty enhanced result of Staduino workshop at Skolska 28 on 26th May 2013.

Consist of 2 sine wave oscillators distorted by one OR value.

Controls
--------
<dl>
	<dt>Learn button</dt>
	<dd>switches between direct mode and MIDI mode</dd>
</dl>

Standuino Controls (in direct mode)
-----------------------------------

<dl>
<dt>Knob 1</dt><dd>Oscillator B volume</dd>
<dt>Knob 2</dt><dd>Oscillator B frequency</dd>
<dt>Knob 3</dt><dd>Distortion amount</dd>
<dt>Knob 4</dt><dd>Oscilator A base note</dd>
<dt>Knob 5</dt><dd>Oscilator A volume</dd>
<dt>Big buttons</dt><dd>when pushed turn on oscillator A at different pitches relative to base note</dd>
</dl>

MIDI Controls (in MIDI mode)
----------------------------

<dl>
<dt>note on</dt><dd>start playing received note on oscillator A</dd>
<dt>note off</dt><dd>stop playing a note on oscillator A</dd>
<dt>aftertouch</dt><dd>controls the oscillator A volume</dd>
<dt>CC 12</dt><dd>frequency of oscillator B</dd>
<dt>CC 13</dt><dd>volume of oscillator B</dd>
<dt>CC 14</dt><dd>distortion amount</dd>
</dl>

Depends on
----------

* [Mozzi library](http://sensorium.github.io/Mozzi/) for sound synthesis
* [FraAngelicoHW](http://www.standuino.eu/) library for easy access to Standuino controls
* [Arduino MIDI Library](http://sourceforge.net/projects/arduinomidilib/) for parsing incoming MIDI messages

License
-------

Copyright (c) 2013 Michal Rydlo

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.
