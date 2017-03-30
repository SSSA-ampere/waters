#!/usr/bin/python

import sys
import csv

from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np

MAX_COST = 0
OLDCOST = 99999

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

filename = sys.argv[1]

counter = 0

with open(filename, 'rb') as csvfile:
	spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
	for row in spamreader:
		COST = float(row[0]) * 5000
		
		if MAX_COST < COST :
                    MAX_COST = COST
		
		RAM_OCCUPATION=[0,0,0,0,0]
		
		del row[0]
		for cell in row:
			RAM_OCCUPATION[int(cell)] = RAM_OCCUPATION[int(cell)] + 1
		
		#print str(RAM_OCCUPATION) + " (" + str(COST) + ")"
		#print row[0]
		#print ",".join(row)
		
		xs = np.arange(6)
		#ys = np.random.rand(6)
		ys = [COST] + RAM_OCCUPATION
		
		
		cs = [  [COST/MAX_COST, 0, 0, 1],
                        [0, RAM_OCCUPATION[0]/MAX_COST, 0, 1],
                        [0, 0, RAM_OCCUPATION[1]/MAX_COST, 1],
                        [RAM_OCCUPATION[2]/MAX_COST, RAM_OCCUPATION[2]/MAX_COST, 0, 1],
                        [0, RAM_OCCUPATION[3]/MAX_COST, RAM_OCCUPATION[3]/MAX_COST, 1],
                        [COST/MAX_COST, 0, 0, 1]  ]
		
		#print cs
		
		print ys
		
		ax.bar(xs, ys, zs=counter, zdir='y', alpha=1)
		counter = counter + 1
		
		if COST > OLDCOST :
                    print "------------ERROR"
                    sys.exit()
		
		OLDCOST = COST

ax.set_xlabel('Cost + RAM_ID')
ax.set_ylabel('Epoch')
ax.set_zlabel('Cost/Allocation')

plt.show()

