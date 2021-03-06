import multiprocessing
import subprocess
import os
import time
import numpy as np
import json

coreCount = multiprocessing.cpu_count()
startPath = '/home/erol/Documents/preach_sampling/'
random = startPath + 'testing/preachRandom'
optimized = startPath + 'testing/preachOptimized'
outputFile = 'heuristicResults.txt'
configFile = '/home/erol/Documents/preach_sampling/testing/configuration.json'

# this function finds sets of target, source, and graph files
def loadData():
    testingData = []
    fileNames = os.listdir(testDataLocation)
    execNames = [file for file in fileNames if '.txt' in file]
    for fileName in execNames:
        sourceName = fileName[:-3] + 'sources'
        targetName = fileName[:-3] + 'targets'
        if (targetName in fileNames) and (sourceName in fileNames):
            # this is a temporary filter
            if int(fileName.split('_')[2]) > 30:
                continue
            ############################
            print(fileName)
            testingData.append( (fileName, sourceName, targetName) )
    return testingData

# this function tests a series of data sets with a given algorithm
def runTests(execFile, data, lock):
    file = open(outputFile, 'a')
    graphFile = testDataLocation + data[0]
    #if (graphFile != testDataLocation + 'BA_2_10_4.txt') and (graphFile != testDataLocation + 'BA_2_20_2.txt'):
    #    return
    temp = open((graphFile[:-4] + "_pipe.txt"), "w")
    print('start')
    sourceFile = testDataLocation + data[1]
    targetFile = testDataLocation + data[2]
    print(graphFile)
    # run program
    p = subprocess.Popen([execFile, graphFile, sourceFile, targetFile, '0', str(testCycles), 'rand', '2', '10'], stdout=temp)
    temp.close()
    temp = open((graphFile[:-4] + "_pipe.txt"), "r")
    buffer = ''
    while True:
        time.sleep(0.5)
        output = str(temp.read()).strip()
        if output:
            #print(output)
            buffer += output
            if '@@@' in buffer:
                print('done')
                break
    timeList = parse(buffer)
    print(timeList)
    if timeList:
        # write to output file
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
    workload = int(np.ceil(float(len(tasks)) / coreCount))
    print(len(tasks), workload)
    lock = multiprocessing.Lock()
    taskGroups = []
    counter = 0
    while counter < len(tasks):
        if (len(tasks) - counter) < workload:
            taskGroups.append(tasks[counter:])
            break
        else:
            #print(counter, (counter+workload))
            taskGroups.append(tasks[counter:(counter+workload)])
            counter += workload
    for group in taskGroups:
        multiprocessing.Process(target=task, args=(group, lock)).start()

# defines the procedure for a specific task
def task(dataList, lock):
    for item in dataList:
        runTests(optimized, item, lock)
        runTests(random, item, lock)

if __name__ == '__main__':
    # read configuatiaon file
    config = open(configFile, 'r')
    configData = json.loads(config.read())
    testCycles = configData['testCycles']
    testDataLocation = startPath + configData['dataPath']
    # run tests
    print(coreCount)
    data = loadData()
    distributeTasks(data)
