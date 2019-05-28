#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
from matplotlib.figure import figaspect

import numpy as np
plt.style.use('seaborn-ticks')

# w,h = figaspect(2/3)

fig, ax = plt.subplots(3, 1, sharex=True)
fig.tight_layout()


orders = [2, 4, 6, 8 ,10 ,12]

#Cache misses

miss_grid_64 = [204994, 352512, 508672, 672768, 814240, 951488]
miss_grid_128 = [1606018, 2720256, 3868160, 5048320, 6138176, 7211392]
miss_grid_256 = [12714754, 21365760, 30149632, 56297984, 90538368, 107414016]

# line_32 = ax.plot(a, grid_32, marker='s', label='32x32x32 Elements')
# line_48 = ax.plot(a, grid_48, marker='^', label='48x48x48 Elements')
# line_64 = ax.plot(a, grid_64, marker='o', label='64x64x64 Elements')
# line_128 = ax.plot(orders, grid_128, marker='D', label='Grid 128x128x128')

line_64 = ax[0].plot(orders, miss_grid_64, marker='s', color='#007ACC',label='Grid 64x64x64')
line_128 = ax[1].plot(orders, miss_grid_128, marker='^', color='#f18e3a',label='Grid 128x128x128')
line_256 = ax[2].plot(orders, miss_grid_256, marker='o', color='#55885e',label='Grid 256x256x256')

plt.xlabel('Stencil Order', fontsize = 12)

ax[0].legend()
ax[1].legend()
ax[2].legend()
ax[0].tick_params(labelsize = 12)
ax[1].tick_params(labelsize = 12)
ax[2].tick_params(labelsize = 12)

ax[0].ticklabel_format(style='sci', axis='y', scilimits=(0,0))
ax[1].ticklabel_format(style='sci', axis='y', scilimits=(0,0))
ax[2].ticklabel_format(style='sci', axis='y', scilimits=(0,0))

ax[0].grid(linestyle='--')
ax[1].grid(linestyle='--')
ax[2].grid(linestyle='--')
ax[0].yaxis.offsetText.set_fontsize(12)
ax[1].yaxis.offsetText.set_fontsize(12)
ax[2].yaxis.offsetText.set_fontsize(12)

plt.tight_layout()
plt.savefig('cacheMissesCmp.eps', format='eps', dpi=1000)
# plt.show()
