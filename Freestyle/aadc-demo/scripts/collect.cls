# simple demo to test connection between Emotiv and CLSquare

[Main]
num_episodes         = 1
cycles_per_episode   = 1000000
sleep_every_cycle    = 10

plant                = EchoPlant
controller           = CenterControl
graphic              = GnuplotterGraphic
observer             = ChainObserver
output               = DefaultOutput

[Output]
output_mode          = standard
output_file          = log/eeg.raw.prot

[Input]
input_mode           = random
xinit                = []

[Controller]
mode                 = Static
value                = 0

[Plant]
actions              = 1
states               = 1

[Graphic]
sources              = o o o o o o o o o o o o o o
indices              = 0 1 2 3 4 5 6 7 8 9 10 11 12 13
line_color           = 0 1 2 3 4 5 6 7 8 9 10 11 12 13
line_type            = 1 1 1 1 1 1 1 1 0 0 0  0  0  0
line_width           = 1 1 1 1 1 1 1 1 2 2 2  2  2  2
# F3, FC6, P7, T8, F7, F8, T7, P8, AF4, F4, AF3, O2, O1, FC5
title_0              = 0 (F3)
title_1              = 1 (FC6)
title_2              = 2 (P7)
title_3              = 3 (T8)
title_4              = 4 (F7)
title_5              = 5 (F8)
title_6              = 6 (T7)
title_7              = 7 (P8)
title_8              = 8 (AF4)
title_9              = 9 (F4)
title_10             = 10 (AF3)
title_11             = 11 (O2)
title_12             = 12 (O1)
title_13             = 13 (FC5)
window_size          = 500
yrange               = -600 600
xrange               = 0 500

[Observer]
observer_0           = EmotivObserver Emotiv
observer_1           = ShuffleObserver ChannelChooser
observer_2           = FilterObserver HighPass
observer_3           = FilterObserver LowPass

[Emotiv]
connection           = hid
# 0   1   2    3   4   5   6   7   8   9    10   11  12  13
# F3, FC6, P7, T8, F7, F8, T7, P8, AF4, F4, AF3, O2, O1, FC5

# F3 (1), F4 (10): no EOG, very weak blink (-)
# FC6 (2), FC5 (14): weak EOG (6L-, 5L+), very weak blink (-)
# P7 (3), P8 (8): no electrode
# T8 (4), T7 (7): no electrode
# F7 (5), F8 (6): strong EOG (7L+, 8L-), medium blink (-)
# AF4 (9), AF3 (11): strong EOG (4L-, 3L+), strong blink (+)
# O2 (12), O1 (13): no electrode

[ChannelChooser]
#assignment          = 8 10 9 0 5 4 1 13
# offset: 1
#assignment           = 9 11 10 1 6 5 2 14 4 7 3 8 12 13
assignment           = 1 2 3 4 5 6 7 8 9 10 11 12 13 14

[HighPass]
mode                 = highpass
inputs               = 0 1 2 3 4 5 6 7 8 9 10 11 12 13
initial              = 1000
delta_t              = 0.01
cutoff_frequency     = 0.1

[LowPass]
mode                 = lowpass
inputs               = 0 1 2 3 4 5 6 7 8 9 10 11 12 13
delta_t              = 0.01
cutoff_frequency     = 1

