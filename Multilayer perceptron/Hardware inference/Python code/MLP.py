import os
import serial
import pandas as pd
import time
from sklearn.metrics import accuracy_score, confusion_matrix
import numpy as np

# Get the path of the current script directory
current_directory = os.path.dirname(os.path.abspath(__file__))

# Build the complete path for dataset.csv file
dataset_file_path = os.path.join(current_directory, 'dataset full.csv')

dataset = pd.read_csv(dataset_file_path)

# Initialize serial communication with Arduino
ser = serial.Serial('COM13', 115200, timeout=1)  # Replace 'COM3' with your Arduino's serial port
time.sleep(0.1)  # Wait for Arduino to reset and get ready

# Wait for Arduino to send a ready signal
while True:
    if ser.readline().decode().strip() == "Ready":
        break

# Prepare experiment data file
experiment_data = []

def send_voltage(x, y):
    ser.write(f"{x},{y}\n".encode())

ser.flushInput()

for index, row in dataset.iterrows():
    send_voltage(row['x'], row['y'])
    time.sleep(0.01) 
    response = ser.readline().decode().strip()

    try:
        real, raw_voltage = response.split(',')
        real = int(real)
        raw_voltage = float(raw_voltage)
        experiment_data.append([row['x'], row['y'], row['label'], real, raw_voltage])
    except ValueError:
        print(f"Error in response: '{response}'")

    if index % 10 == 0:
        print(f"Processed {index} rows")

experiment_df = pd.DataFrame(experiment_data, columns=['x', 'y', 'label', 'real', 'raw'])

# Calculate accuracy and confusion matrix
actual_labels = experiment_df['label']
predicted_labels = experiment_df['real']

accuracy = accuracy_score(actual_labels, predicted_labels)
conf_matrix = confusion_matrix(actual_labels, predicted_labels)

# Add accuracy and confusion matrix to DataFrame
experiment_df['accuracy'] = accuracy
experiment_df['confusion_matrix'] = str(conf_matrix.tolist())

# Print accuracy and confusion matrix
print("Inference Accuracy:", accuracy)
print("Confusion Matrix:\n", conf_matrix)

# Save results to CSV file
experiment_file_path = os.path.join(current_directory, 'experiment.csv')
experiment_df.to_csv(experiment_file_path, index=False)

ser.close()
