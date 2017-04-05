#!/usr/bin/python

import sys
import csv

from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np

MAX_COST = 0
OLDCOST = 99999

fig = plt.figure(1)

#p1 = fig.add_subplot(211, projection='3d')
p1 = fig.add_subplot(211)
p2 = fig.add_subplot(212)

filename = sys.argv[1]

counter = 0

MAX_RAM = 0
MIN_RAM = 10000

lram0 = []
lram1 = []
lram2 = []
lram3 = []
gram = []

epoch = []

y2 = []

with open(filename, 'rb') as csvfile:
	spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
	for row in spamreader:
		COST = float(row[0]) * 5000
		
		y2.append(float(row[0]))
		
		#if MAX_COST < COST :
                    #MAX_COST = COST
                
                epoch.append(int(row[1]))
                
		RAM_OCCUPATION=[0,0,0,0,0]
		
		del row[0]
		del row[0]
		for cell in row:
                    RAM_OCCUPATION[int(cell)] = RAM_OCCUPATION[int(cell)] + 1
                
                lram0.append(RAM_OCCUPATION[0])
                lram1.append(RAM_OCCUPATION[1])
                lram2.append(RAM_OCCUPATION[2])
                lram3.append(RAM_OCCUPATION[3])
                gram.append(RAM_OCCUPATION[4])
                
                for i in range(0, 5) :
                    if RAM_OCCUPATION[i] > MAX_RAM :
                        MAX_RAM = RAM_OCCUPATION[i]
                    
                    if RAM_OCCUPATION[i] < MIN_RAM :
                        MIN_RAM = RAM_OCCUPATION[i]
		
		
		##print str(RAM_OCCUPATION) + " (" + str(COST) + ")"
		##print row[0]
		##print ",".join(row)
		
		#xs = np.arange(5)
		##ys = np.random.rand(6)
		##ys = [COST] + RAM_OCCUPATION
		#ys = RAM_OCCUPATION
		
		
		
		
		
		#cs = [  [0, RAM_OCCUPATION[0]/MAX_COST, 0, 1],
                        #[0, 0, RAM_OCCUPATION[1]/MAX_COST, 1],
                        #[RAM_OCCUPATION[2]/MAX_COST, RAM_OCCUPATION[2]/MAX_COST, 0, 1],
                        #[0, RAM_OCCUPATION[3]/MAX_COST, RAM_OCCUPATION[3]/MAX_COST, 1],
                        #[COST/MAX_COST, 0, 0, 1]  ]
		
		##print cs
		
		#print str(float(row[0])) + " : " + str(ys)
		
		#p1.bar(xs, ys, zs=counter, zdir='y', alpha=1)
		#counter = counter + 1
		
		#if COST > OLDCOST :
                    #print "------------ERROR"
                    #sys.exit()
		
		#OLDCOST = COST


#epoch = np.arange(0, len(y2), 1)


line1 = p1.plot(epoch, lram0, 'k', label="LRAM0")
line2 = p1.plot(epoch, lram1, 'b', label="LRAM1")
line3 = p1.plot(epoch, lram2, 'c', label="LRAM2")
line4 = p1.plot(epoch, lram3, 'g', label="LRAM3")
line5 = p1.plot(epoch, gram, 'r', label="GRAM")

p1.set_ylim([MIN_RAM, MAX_RAM])
p1.set_xlim([0, epoch[len(epoch) - 1]])
p1.set_xlabel('Epoch')
p1.set_ylabel('Memory occupation')
#p1.set_zlabel('Cost/Allocation')

p1.legend()
p1.grid()

given_solution_result = 1.32025

p2.plot(epoch, y2, 'k')
p2.plot(epoch, [given_solution_result] * len(y2), 'r--')
p2.plot(epoch, [1] * len(y2), 'b--')

FIT_MIN = y2[len(y2) - 1] if y2[len(y2) - 1] < 1 else 1
FIT_MAX = y2[0] if y2[0] > given_solution_result else given_solution_result

p2.set_xlim([0, epoch[len(epoch) - 1]])
p2.set_ylim(FIT_MIN, FIT_MAX)

extraticks = [FIT_MIN, FIT_MAX]
p2.set_yticks(list(p2.get_yticks()) + extraticks)

p2.set_ylabel('Fitness')
p2.set_xlabel('Epoch')
p2.grid()

plt.show()

