local M = {}

M.NOTE_NAMES = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}

M.NOTE_NAMES_SOLFEGE = {"Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa", "Sol", "Sol#", "La", "La#", "Si"}

M.octave_modifier = 1
-- 1 matches 60 / middle C to C4
-- 2 matches 60 / middle C to C3 and so on

function M.number_to_name(note_number, include_octave)
	local name  = M.NOTE_NAMES[note_number % 12 + 1]
	if include_octave then 
		name = name .. math.floor(note_number / 12 - M.octave_modifier)
	end
	return name
end

function M.number_to_name_solfege(note_number, include_octave)
	local name  = M.NOTE_NAMES_SOLFEGE[note_number % 12 + 1]
	if include_octave then 
		name = name .. " " .. math.floor(note_number / 12 - M.octave_modifier)
	end
	return name
end

function M.number_to_frequency(note_number)
	return 13.75 * (2 ^ ((note_number - 9) / 12 - M.octave_modifier + 1))
end

return M