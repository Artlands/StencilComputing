#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter
from matplotlib.figure import figaspect
import numpy as np
plt.style.use('seaborn-ticks')


orders = [2, 4, 6, 8 ,10 ,12]
posi1 = [1.6, 3.6, 5.6, 7.6 ,9.6 ,11.6]
posi2 = [2.4, 4.4, 6.4, 8.4 ,10.4 ,12.4]

host_256 = [41746247, 48173004, 54448525, 73161010, 96869258, 107881214]
pims_256 = [35850687, 36008948, 36166980, 36407512, 36735970, 37048191]

host_128 = [5231917, 6045641, 6883363, 7723892, 8464174, 9191275]
pims_128 = [4535988, 4512631, 4537765, 4567981, 4605759, 4655403]

host_64 = [656252, 764761, 877123, 996275, 1087851, 1173146]
pims_64 = [556268, 557294, 564670, 575560, 580624, 587294]

redu_256 = []
redu_128 = []
redu_64 = []

for i in range(0,6):
    re_256 = (host_256[i] - pims_256[i]) * 100/ host_256[i]
    redu_256.append(re_256)

    re_128 = (host_128[i] - pims_128[i]) * 100 / host_128[i]
    redu_128.append(re_128)

    re_64 = (host_64[i] - pims_64[i]) * 100 / host_64[i]
    redu_64.append(re_64)

# print(redu_256)
# print(redu_128)

ind = np.arange(len(orders))
x = 2*ind + 2
width = 0.3

w,h = figaspect(1/2)
fig, ax = plt.subplots(figsize=(w,h))

# ax.bar(ind-width/2 - 0.01, redu_256, width, color='#DF6466')
# # ax.bar(ind-width/2 - 0.01, host_d, bottom = host_c, color='#f7ac8b', width = width)

# ax.bar(ind+width/2 + 0.01, redu_128, width, color='#676488')
# # ax.bar(ind+width/2 + 0.01, pims_d, bottom = pims_c, color='#ACC0D3', width = width)

ax.vlines(posi1, ymin=0, ymax=redu_64, color='#7badce', alpha=1, linewidth=18)
ax.plot(posi1, redu_64,"o", markersize=18, color='#007ACC', alpha=1, label = "Grid 64x64x64")

ax.vlines(orders, ymin=0, ymax=redu_128, color='#edbc92', alpha=1, linewidth=18)
ax.plot(orders , redu_128,"o", markersize=18, color='#f18e3a', alpha=1, label = "Grid 128x128x128")

ax.vlines(posi2, ymin=0, ymax=redu_256, color='#8bae91', alpha=1, linewidth=18)
ax.plot(posi2, redu_256,"o", markersize=18, color='#55885e', alpha=1, label = "Grid 256x256x256")

plt.xlabel('Stencil Order', fontsize = 16)
plt.ylabel('Bank Conflicts Reduction(%)', fontsize = 16)

plt.xticks(fontsize=16)
plt.yticks(fontsize=16)
# plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))

vals = ax.get_yticks()
ax.set_yticklabels(['{:,.2f}'.format(x) for x in vals])

ax.xaxis.set_ticks(orders)
ax.xaxis.set_ticklabels(orders)
ax.grid(linestyle='--')
ax.yaxis.offsetText.set_fontsize(14)
ax.legend(fontsize=14)



plt.tight_layout()
plt.savefig('ThroughputReduction.eps', format='eps', dpi=1000)
# plt.show()
