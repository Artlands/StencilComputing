#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter
from matplotlib.figure import figaspect
import numpy as np
plt.style.use('seaborn-ticks')


orders = [2, 4, 6, 8 ,10 ,12]

# Request
host = [8, 14, 20, 26, 32, 38]
pims_h = [2,2,2,2,2,2]
pims_p = [1,2,3,4,5,6]
pims = [3,4,5,6,7,8]

ctrl = 16

# Throughput
host_t = []
host_d = []
host_c = []
pims_t = []
pims_d = []
pims_c = []

for i in range(0,6):
    d_h = host[i] * 800/(host[i] * 8 + host[i] * 16)
    host_d.append(d_h)
    c_h = host[i] * 1600/(host[i] * 8 + host[i] * 16)
    host_c.append(c_h)
    host_t.append(d_h+c_h)

    d_p = pims[i] * 800/(host[i] * 8 + host[i] * 16)
    pims_d.append(d_p)
    c_p = pims[i] * 1600/(host[i] * 8 + host[i] * 16)
    pims_c.append(c_p)
    pims_t.append(d_p+c_p)


print(host_d)
print(host_c)
print(pims_d)
print(pims_c)

ind = np.arange(len(orders))
x = 2*ind + 2
width = 0.3

w,h = figaspect(1/2)
fig, ax = plt.subplots(figsize=(w,h))

ax.bar(ind-width/2 - 0.01, host_c, width, color='#DF6466')
ax.bar(ind-width/2 - 0.01, host_d, bottom = host_c, color='#f7ac8b', width = width)

ax.bar(ind+width/2 + 0.01, pims_c, width, color='#676488')
ax.bar(ind+width/2 + 0.01, pims_d, bottom = pims_c, color='#ACC0D3', width = width)

# rects1 = ax.bar(ind, red_grid_64, width, label = "Grid 128x128x128")
# rects2 = ax.bar(ind, red_grid_128, width, label = "Grid 128x128x128")
# rects3 = ax.bar(ind + width, red_grid_256, width, label = "Grid 256x256x256")

plt.xlabel('Stencil Order', fontsize = 16)
plt.ylabel('Throughput Reduction(%)', fontsize = 16)

plt.xticks(fontsize=16)
plt.yticks(fontsize=16)

vals = ax.get_yticks()
ax.set_yticklabels(['{:,.2f}'.format(x) for x in vals])

ax.xaxis.set_ticks(ind)
ax.xaxis.set_ticklabels(x)
ax.yaxis.grid(linestyle='--')
ax.yaxis.offsetText.set_fontsize(14)
# ax.legend(fontsize=14)



plt.tight_layout()
plt.savefig('ThroughputReduction.eps', format='eps', dpi=1000)
# plt.show()
