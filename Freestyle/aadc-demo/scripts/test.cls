# simple demo to test connection between Emotiv and CLSquare

[Main]
num_episodes             = 1
cycles_per_episode       = 1000000
sleep_every_cycle        = 100

plant                    = MultiPlant
controller               = MixControl
graphic                  = GnuplotterGraphic
observer                 = EOGObserver
output                   = DefaultOutput

[Output]
output_mode              = standard
output_file              = log/test.prot

[Input]
input_mode               = random
xinit                    = [][][][]

[Controller]
controller_0             = EOG2Tapir Direction
action_dim_0             = 1
state_dim_0              = 1
state_assign_0           = 5
assignment               = [0 0]

[Direction]
actions                  = 0 1 2

[Kuka]

[Plant]
plant_0                  = TapirPlant Vision

[Vision]
cam_0                    = vision/tapir.cfg
$ leftright.cls

[Graphic]
sources                  = o o
indices                  = 4 5
line_color               = 1 2
title_0                  = Blink Event
title_1                  = EOG Event
window_size              = 500
yrange                   = -1.5 1.5
xrange                   = 0 500
custom_script            = scripts/plot.gp

[Observer]
connection               = hid
config                   = config/eog.cfg
ica                      = true

