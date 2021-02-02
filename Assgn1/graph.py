
import matplotlib.pyplot as plt 
  
# line 1 points 
x1 = [10,11,12,13,14,15] 
y1 = [10,11,12,13,14,15] 
#y1 = [6 ,6.8 ,9.1,10, 9.2 , 9.5 , 13 ,3.5  , 4 ,12 ,5.3 ]
# plotting the line 1 points add label name corresponding to this curve
plt.plot(x1, y1,'bo-', label = "Vector Clock") 
  
# line 2 points 
x2 = [10,11,12,13,14,15] 
y2 = [1.374,1.53,1.76,1.91,2.13,2.37] 
#y2 = [3.2 , 17 , 21, 42, 64 , 26 , 27 , 20 ,18 , 16 , 12]
# plotting the line 2 points  
plt.plot(x2, y2, 'ro-',label = "Optimized Clock") 

# x3 = [5,10,15,20,25,30,35] 
# y3 = [20.35,20.29,20.26,20.14,20.03,21.2,21.3] 
# # plotting the line 1 points  
# plt.plot(x3, y3,'yo-', label = "SAM2") 
  
# # line 4 points 
# x4 = []
# y4 = [] 
# # plotting the line 4 points  
# plt.plot(x4, y4, 'go-',label = ")") 


  
# naming the x axis 
plt.ylabel('Average number of entries sent per message') 
# naming the y axis 
plt.xlabel('No of Threads') 
# giving a title to my graph 
plt.title('Average number of entries sent per message Full Network Topology')	 
  
# show a legend on the plot 
plt.legend(loc='upper left') 
  
# function to show the plot 
plt.show() 
