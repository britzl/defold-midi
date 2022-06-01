# Defold MIDI
Get MIDI input in Defold from MIDI devices.

This project uses [RtMidi](https://github.com/thestk/rtmidi). If you need more features that RtMidi supports you can either create an issue to request the feature, or you can implement the new features yourself and make a pull request for code changes.

Currently supported platforms:
* Windows
* macOS
* HTML5

More platforms are possible to add such as Linux and iOS but these are not tested yet. To enable these for testing you will need to modify the code of this native extension.

# Installation
You can use Defold MIDI in your own project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:

	https://github.com/subsoap/defold-midi/archive/master.zip

Or point to the ZIP file of a [specific release](https://github.com/subsoap/defold-midi/releases).


# Usage

```
local function initialize_midi()
	print("initialize_midi")
	local out_count = midi.count_out()
	local in_count = midi.count_in()
	print("out#", out_count)
	print("in#", in_count)

	if in_count > 0 then
		midi.open_in(0, "foo");
		print("In open:", midi.is_in_open())
		print("Out open:", midi.is_out_open())

		print(midi.get_name_in(0))
	else
		print("No input devices")
	end
end

function init(self)
	midi.set_listener(function(self, message)
		if message.event == midi.EVENT_MESSAGE then
			print("MESSAGE", self.frame)
			pprint(message.data)
		elseif message.event == midi.EVENT_ERROR then
			print("ERROR " .. message.error)
		elseif message.event == midi.EVENT_READY then
			initialize_midi()
		end
	end)
end
```

# Helpers 

If you are making a MIDI game or tool you may find the following files useful:

## midi/helpers/midi_map.lua

A hard coded list of MIDI codes to notes. C4 is mapped to MIDI code 60.

```
local midi_map = require("midi.helpers.midi_map")

local middle_c_note_english = midi_map[60].note_name

...

midi_note_number = 60,
keys_61 = 25,
keys_88 = 40,
note_name = "C4",
is_middle_c = true,
frequency = 261.63

```

## midi/helpers/midi_tools.lua

Functions to dynamically convert MIDI codes to English notes, solfège syllables, and frequencies. 

```
local midi_tools = require("midi.helpers.midi_tools")

local middle_c_note_english = midi_tools.number_to_name(60, true)
local middle_c_note_solfege = midi_tools.number_to_name_solfege(60, true)
local middle_c_note_frequency = midi_tools.number_to_frequency(60)
```

These functions assume that MIDI code 60 is C4. You can adjust this by modifying midi_tools.octave_modifier to be higher or lower.