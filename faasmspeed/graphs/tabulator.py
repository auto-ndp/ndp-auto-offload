#!/usr/bin/python3
import sys
import os


def main():
    if len(sys.argv) < 3:
        print("Usage: tabulator.py input output")
        return
    inputPath = sys.argv[1]
    outputPath = sys.argv[2]
    lines = []
    with open(inputPath, 'r') as file:
        lines = file.readlines()
    lines = list(map(lambda l: l.strip(), lines))
    outputFile = open(outputPath, 'w')
    # detect fields
    fields = []
    for line in lines:
        csv = list(line.split(','))
        if len(fields) != 0 and csv[0] == 'opts':
            break
        varHeader = csv[0].strip()
        for i in range(1, len(csv), 2):
            varLabel = csv[i]
            varName = f"{varHeader}_{varLabel}"
            fields.append(varName)
    outputFile.write(
        ','.join(map(lambda f: f.replace('-', '_'), fields)) + ',dummy')
    fieldNo = 0
    fieldHead = 'pre-fields'
    for line in lines:
        csv = list(line.split(','))
        varHeader = csv[0].strip()
        if varHeader == 'opts':
            fieldNo = 0
            fieldHead = line
            outputFile.write("\n")
        for i in range(1, len(csv)-1, 2):
            varLabel = csv[i]
            varName = f"{varHeader}_{varLabel}"
            varValue = csv[i+1]
            if fields[fieldNo] != varName:
                print(
                    f"Warning: skipping unknown field {varName}={varValue} at datapoint {fieldHead}")
            else:
                outputFile.write(f"\"{varValue}\",")
                fieldNo += 1
    outputFile.flush()
    outputFile.close()


if __name__ == '__main__':
    main()
