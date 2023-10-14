import os
os.system("gcc ./code2.c -o code2")
for i in range(1,6):
  print("run time" + str(i))
  os.system("./code2")
print("\n")