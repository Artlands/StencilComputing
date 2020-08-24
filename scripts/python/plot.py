#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
import numpy as np
plt.style.use('seaborn-ticks')

fig, ax = plt.subplots()

# orders = [2, 4, 6, 8 ,10 ,12]
a = np.arange(5, 14)
x = 2**a
# print(x)
#Cache misses
grid_32 = [94720, 90952, 22374, 15438, 15360, 15360, 15360, 15360, 15360]
grid_48 = [295680, 295680, 273385, 96919, 44114, 43008, 43008, 43008, 43008]
grid_64 = [672768, 672768, 672768, 551697, 158618, 93732, 92160, 92160, 92160]
# grid_128 = [1606018, 2720256, 3868160, 5048320, 6138176, 7211392]
# grid_256 = [12714754, 22714754, 32714754, 42714754, 52714754, 62714754]

line_32 = ax.plot(a, grid_32, marker='s', label='Grid 32x32x32')
line_48 = ax.plot(a, grid_48, marker='^', label='Grid 48x48x48')
line_64 = ax.plot(a, grid_64, marker='o', label='Grid 64x64x64')
# line_128 = ax.plot(orders, grid_128, marker='D', label='Grid 128x128x128')
# line_256 = ax.plot(orders, grid_256, marker='^', label='Grid 256x256x256')
plt.xlabel('Cache Size in KB', fontsize = 16)
plt.ylabel('Cache misses', fontsize = 16)
plt.legend(['Grid 32x32x32','Grid 48x48x48','Grid 64x64x64'],fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))

ax.xaxis.set_ticks(a)
ax.xaxis.set_ticklabels(x)
ax.grid(linestyle='--')
ax.yaxis.offsetText.set_fontsize(14)
plt.savefig('cacheMisses.eps', format='eps', dpi=1000)
# plt.show()
