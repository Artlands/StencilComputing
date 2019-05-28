#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter
from matplotlib.figure import figaspect
import numpy as np
plt.style.use('seaborn-ticks')

orders = [2, 4, 6, 8 ,10 ,12]

#Stencil size
size_grid_64 = 262144
size_grid_128 = 2097152
size_grid_256 = 16777216

#Memory traffic
host_grid_32 = [626944, 3022848, 4481024, 6062080, 7279616, 8427520]
host_grid_64 = [13119616, 22560768, 32555008, 43057152, 52111360, 60895232]
host_grid_128 = [102785152, 174096384, 247562240, 323092480, 392843264, 461529088]
host_grid_256 = [813744256, 1367408640, 1929576448, 3603070976, 5794455552, 6874497024]

pims_grid_32 = [1083392, 1638400, 2196480, 2752512, 3260416, 3768320]
pims_grid_64 = [8523776, 12845056, 17172480, 21495808, 25624576, 29753344]
pims_grid_128 = [67641344, 101711872, 135794688, 169869312, 203161600, 236453888]
pims_grid_256 = [538984448, 809500672, 1080041472, 1350565888, 1617952768, 1885339648]

# Memory traffic per point
host_point_64 = []
host_point_128 = []
host_point_256 = []
pims_point_64 = []
pims_point_128 = []
pims_point_256 = []

for i in range(0,6):
    host_point_64.append(host_grid_64[i]/size_grid_64)
    host_point_128.append(host_grid_128[i]/size_grid_128)
    host_point_256.append(host_grid_256[i]/size_grid_256)
    pims_point_64.append(pims_grid_64[i]/size_grid_64)
    pims_point_128.append(pims_grid_128[i]/size_grid_128)
    pims_point_256.append(pims_grid_256[i]/size_grid_256)

# Memory traffic reduction
red_grid_32 = []
red_grid_64 = []
red_grid_128 = []
red_grid_256 = []
for i in range(0,6):
    red_32 = (host_grid_32[i] - pims_grid_32[i]) * 100/host_grid_32[i]
    red_grid_32.append(red_32)

    red_64 = (host_grid_64[i] - pims_grid_64[i])* 100/host_grid_64[i]
    red_grid_64.append(red_64)

    red_128 = (host_grid_128[i] - pims_grid_128[i])* 100/host_grid_128[i]
    red_grid_128.append(red_128)

    red_256 = (host_grid_256[i] - pims_grid_256[i])* 100/host_grid_256[i]
    red_grid_256.append(red_256)

# print(red_grid_64)

ind = np.arange(len(orders))
width = 0.2

w,h = figaspect(1/2)
fig, ax = plt.subplots(figsize=(w,h))

# rects0 = ax.bar(ind - 2*width, red_grid_32, width, label = "Grid 32x32x32")
rects1 = ax.bar(ind - width, red_grid_64, width, label = "Grid 64x64x64")
rects2 = ax.bar(ind, red_grid_128, width, label = "Grid 128x128x128")
rects3 = ax.bar(ind + width, red_grid_256, width, label = "Grid 256x256x256")

plt.xlabel('Stencil Order', fontsize = 16)
plt.ylabel('Memory Traffic Reduction(%)', fontsize = 16)

plt.xticks(fontsize=16)
plt.yticks(fontsize=16)
# plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
vals = ax.get_yticks()
ax.set_yticklabels(['{:,.2f}'.format(x) for x in vals])

ax.xaxis.set_ticks(ind)
ax.xaxis.set_ticklabels(orders)
ax.grid(linestyle='--')
ax.yaxis.offsetText.set_fontsize(14)
ax.legend(fontsize=14)
plt.tight_layout()
plt.savefig('MemoryTrafficReduction.eps', format='eps', dpi=1000)
# plt.show()
