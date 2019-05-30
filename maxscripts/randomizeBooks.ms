--Loop through selected objects
for i = 1 to selection.count do (
	--randomize the position, scale, and rotation
	selection[i].scale *= (random .95 1.05)
	selection[i].rotation = eulerangles(random -.5 .5) (random -.5 .5) (random -.5 .5)
	selection[i].pos.x += random -.002 0.0
	selection[i].pos.y += random 0.0 .04
)