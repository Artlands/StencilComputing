#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter
from matplotlib.figure import figaspect
import numpy as np
plt.style.use('seaborn-ticks')


orders = [2, 4, 6, 8 ,10 ,12]

host_256 = [41746247, 48173004, 54448525, 73161010, 96869258, 107881214]
pims_256 = [35850687, 36008948, 36166980, 36407512, 36735970, 37048191]

ind = np.arange(len(orders))
x = 2*ind + 2
width = 0.3

w,h = figaspect(1/2)
fig, ax = plt.subplots(figsize=(w,h))

ax.bar(ind-width/2 - 0.01, host_256, width, color='#DF6466', label = "Without PIMS")

ax.bar(ind+width/2 + 0.01, pims_256, width, color='#676488', label = "With PIMS")

# rects1 = ax.bar(ind, red_grid_64, width, label = "Grid 128x128x128")
# rects2 = ax.bar(ind, red_grid_128, width, label = "Grid 128x128x128")
# rects3 = ax.bar(ind + width, red_grid_256, width, label = "Grid 256x256x256")

plt.xlabel('Stencil Order', fontsize = 16)
plt.ylabel('Bank Conflicts', fontsize = 16)
plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))

plt.xticks(fontsize=16)
plt.yticks(fontsize=16)

vals = ax.get_yticks()
# ax.set_yticklabels(['{:,.2f}'.format(x) for x in vals])

ax.xaxis.set_ticks(ind)
ax.xaxis.set_ticklabels(x)
ax.yaxis.grid(linestyle='--')
ax.yaxis.offsetText.set_fontsize(14)
ax.legend(fontsize=14)



plt.tight_layout()
plt.savefig('BankConflicts256.eps', format='eps', dpi=1000)
# plt.show()
