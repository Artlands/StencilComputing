#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib
matplotlib.use('TkAgg')

import matplotlib.pyplot as plt
import numpy as np

from matplotlib.ticker import PercentFormatter

# Data
req_raw = [3,5,5,9,7,27]
resp_raw = [3,5,5,9,7,27]
req_pim = [1,1,1,1,1,1]
resp_pim = [2,3,2,3,2,4]

raw_pct = []
req_pct = []
resp_pct = []

for i in range(0, 6):
    raw_pct.append(req_raw[i]/(req_raw[i] + resp_raw[i]))
    req_pct.append(req_pim[i]/(req_raw[i] + resp_raw[i]))
    resp_pct.append(resp_pim[i]/(req_raw[i] + resp_raw[i]))

print(raw_pct)
print(req_pct)
print(resp_pct)

# Set position of bar on X axis
barWidth = 0.35
r1 = np.arange(len(raw_pct))
r2 = [x + barWidth for x in r1]

# Create requst bars
plt.bar(r1, raw_pct, bottom = raw_pct, color = '#a3acff',edgecolor = 'white', width = barWidth)
plt.bar(r2, req_pct, bottom = resp_pct, color = '#a3acff',edgecolor = 'white', width = barWidth, label='request')

# Create response bars
plt.bar(r1, raw_pct, color = '#f9bc86', edgecolor = 'white', width = barWidth)
plt.bar(r2, resp_pct, color = '#f9bc86', edgecolor = 'white', width = barWidth, label='response')

# Custom x axis
plt.xticks([r + barWidth/2 for r in range(len(raw_pct))], ['1D-3points','1D-5points','2D-5points','2D-9points','3D-7points','3D-27points'])
plt.xlabel('Stencil type')

plt.ylabel('I/O Reduction')
plt.gca().yaxis.set_major_formatter(PercentFormatter(1))
plt.gca().yaxis.grid(linestyle=':')
# plt.rcParams.update({'font.size': 22})

# Add a legend
plt.legend(loc='upper left', bbox_to_anchor=(1,1), ncol=1)

# Show graphic
# plt.savefig('io_reduction', dpi=200)
plt.show()
