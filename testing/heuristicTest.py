import multiprocessing
import subprocess
import os
import time
import numpy as np

coreCount = multiprocessing.cpu_count()
testCycles = 10
testDataLocation = 'data/synthetic/'
startPath = '/home/erol/Documents/preach_sampling/'
random = 'testing/preachRandom'
optimized = 'testing/preachOptimized'
outputFile = 'heuristicResults.txt'

# this function finds sets of target, source, and graph files
def loadData():
    testingData = []
    fileNames = os.listdir(testDataLocation)
    execNames = [file for file in fileNames if '.txt' in file]
    for fileName in execNames:
        sourceName = fileName[:-3] + 'sources'
        targetName = fileName[:-3] + 'targets'
        if (targetName in fileNames) and (sourceName in fileNames):
            testingData.append( (fileName, sourceName, targetName) )
    return testingData

# this function tests a series of data sets with a given algorithm
def runTests(execFile, data, lock):
    file = open(outputFile, 'a')
    graphFile = startPath + testDataLocation + data[0]
    if graphFile != startPath + testDataLocation + 'BA_2_5_4.txt':
        return
    print('start')
    sourceFile = startPath + testDataLocation + data[1]
    targetFile = startPath + testDataLocation + data[2]
    # run program
    print(graphFile)
    p = subprocess.Popen([execFile, graphFile, sourceFile, targetFile, '0', str(testCycles), 'rand', '2', '10'], stdout=subprocess.PIPE)
    buffer = ''
    while True:
        #print(buffer)
        if p.stdout:
            output = str(p.communicate()[0].strip())
            print(output)
            if output:
                buffer += output
                if '@@@' in buffer:
                    break
    timeList = parse(buffer)
    print(timeList)
    lock.acquire()
    try:
        file.write(str(data[0]) + ',' + execFile + ',' + str(np.mean(timeList)) + ',' + str(np.std(timeList)) + '\n')
        print('writing')
    except:
        print('error')
    finally:
        lock.release()
        pass
    file.close()

# this parses out and organizes the output data
def parse(rawOutput):
    tests = rawOutput.split('---')
    tests = [tests[index] for index in range(len(tests)) if (index % 2)]
    timeList = []
    for test in tests:
        temp = test.split('***')
        time = sum([float(temp[index]) for index in range(len(temp)) if (index % 2)])
        timeList.append(time)
    return timeList

# this distributes the tasks between the cores
def distributeTasks(tasks):
    workload = int(np.ceil(len(tasks) / coreCount))
    lock = multiprocessing.Lock()
    taskGroups = []
    counter = 0
    while counter < len(tasks):
        if (len(tasks) - counter) < workload:
            taskGroups.append(tasks[counter:])
        else:
            print(counter, (counter+workload))
            taskGroups.append(tasks[counter:(counter+workload)])
            counter += workload
    for group in taskGroups:
        multiprocessing.Process(target=task, args=(group, lock)).start()

# defines the procedure for a specific task
def task(dataList, lock):
    for item in dataList:
        runTests('testing/preachRandom', item, lock)
        runTests('testing/preachOptimized', item, lock)

if __name__ == '__main__':
    data = loadData()
    distributeTasks(data)