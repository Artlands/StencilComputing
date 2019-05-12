# #!/usr/bin/env python
# # -*- coding: utf-8 -*-
#

# %matplotlib inline
import matplotlib.pyplot as plt
plt.style.use('seaborn-whitegrid')

orders = [2, 4, 6, 8 ,10 ,12]
#Cache misses
# grid_32 = [0.014764, 0.024067, 0.038086, 0.271325,  0.271325, 0.274054]
# grid_64 = [0.041675, 0.039072, 0.038053, 0.271298,  0.274027, 0.276242]
# grid_128 = [0.041669, 0.039065, 0.038046, 0.333791,  0.328496, 0.321690]
# grid_256 = [0.041667, 0.039063, 0.342477, 0.396615,  0.389358,  0.421165]
#Memory read requests
grid_32 = [4354, 12618, 28704, 266723,  332267, 398322]
grid_64 = [98324, 163878, 229432, 2133571,  2657875, 3186273]
grid_128 = [786468, 1310790,1835112, 21000306,   25489539, 29683874]
grid_256 = [6291524, 10485894, 132153535, 199622848, 241696901, 310902882]

line_32 = plt.plot(orders, grid_32, marker='s', label='Grid 32x32x32')
line_64 = plt.plot(orders, grid_64, marker='o', label='Grid 64x64x64')
line_128 = plt.plot(orders, grid_128, marker='D', label='Grid 128x128x128')
line_256 = plt.plot(orders, grid_256, marker='^', label='Grid 256x256x256')
plt.xlabel('Orders')
plt.ylabel('Cache miss-rate')
plt.ylabel('Memory read requests')
plt.legend(['Grid 32x32x32','Grid 64x64x64','Grid 128x128x128','Grid 256x256x256'])
plt.savefig('memRead.eps', format='eps', dpi=1000)

# import matplotlib
# matplotlib.use('TkAgg')
#
# import matplotlib.pyplot as plt
# import numpy as np
#
# from matplotlib.ticker import PercentFormatter
#
# # Data
# req_raw = [3,5,5,9,7,27]
# resp_raw = [3,5,5,9,7,27]
# req_pim = [1,1,1,1,1,1]
# resp_pim = [2,3,2,3,2,4]
#
# raw_pct = []
# req_pct = []
# resp_pct = []
#
# for i in range(0, 6):
#     raw_pct.append(req_raw[i]/(req_raw[i] + resp_raw[i]))
#     req_pct.append(req_pim[i]/(req_raw[i] + resp_raw[i]))
#     resp_pct.append(resp_pim[i]/(req_raw[i] + resp_raw[i]))
#
# print(raw_pct)
# print(req_pct)
# print(resp_pct)
#
# # Set position of bar on X axis
# barWidth = 0.35
# r1 = np.arange(len(raw_pct))
# r2 = [x + barWidth for x in r1]
#
# # Create requst bars
# plt.bar(r1, raw_pct, bottom = raw_pct, color = '#a3acff',edgecolor = 'white', width = barWidth)
# plt.bar(r2, req_pct, bottom = resp_pct, color = '#a3acff',edgecolor = 'white', width = barWidth, label='request')
#
# # Create response bars
# plt.bar(r1, raw_pct, color = '#f9bc86', edgecolor = 'white', width = barWidth)
# plt.bar(r2, resp_pct, color = '#f9bc86', edgecolor = 'white', width = barWidth, label='response')
#
# # Custom x axis
# plt.xticks([r + barWidth/2 for r in range(len(raw_pct))], ['1D-3points','1D-5points','2D-5points','2D-9points','3D-7points','3D-27points'])
# plt.xlabel('Stencil type')
#
# plt.ylabel('I/O Reduction')
# plt.gca().yaxis.set_major_formatter(PercentFormatter(1))
# plt.gca().yaxis.grid(linestyle=':')
# # plt.rcParams.update({'font.size': 22})
#
# # Add a legend
# plt.legend(loc='upper left', bbox_to_anchor=(1,1), ncol=1)
#
# # Show graphic
# # plt.savefig('io_reduction', dpi=200)
# plt.show()
