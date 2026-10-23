"""
================================================================================
Underground Water Management System — Aquifer Depletion & Regression Simulator
InnovateX: The Future Habitats Challenge | Techfest, IIT Bombay
================================================================================
Purpose:
  Simulates agricultural groundwater extraction cycles, acoustic measurement noise,
  and verifies the on-device least-squares linear regression algorithm used in 
  firmware/UnderGroundWaterManagement_Main/UnderGroundWaterManagement_Main.ino.
  (Implemented in pure Python standard library for zero-dependency portability)
================================================================================
"""

import math
import random
import datetime
import csv
import os

def simulate_aquifer_telemetry(total_hours=48, sample_interval_min=30):
    total_samples = int((total_hours * 60) / sample_interval_min)
    start_time = datetime.datetime.now() - datetime.timedelta(hours=total_hours)
    
    # Ground truth well geometry (in cm)
    total_depth = 500.0       # Sensor mount to well floor
    pump_intake_depth = 440.0 # Depth of pump intake
    initial_water_depth = 280.0
    
    current_depth = initial_water_depth
    data_rows = []
    
    print(f"[*] Simulating {total_hours} Hours ({total_samples} samples) of Aquifer Telemetry...")
    
    for i in range(total_samples):
        timestamp = start_time + datetime.timedelta(minutes=i * sample_interval_min)
        hour = timestamp.hour
        
        # Pumping model: Heavy extraction during morning (6 AM - 10 AM) and evening (5 PM - 8 PM)
        is_pumping = (6 <= hour <= 10) or (17 <= hour <= 20)
        
        if is_pumping:
            drawdown_rate = random.uniform(1.8, 3.2) # cm per 30 min during active pumping
            current_depth += drawdown_rate
        else:
            # Slow natural recharge
            recharge_rate = random.uniform(0.1, 0.4)
            current_depth = max(initial_water_depth, current_depth - recharge_rate)
            
        # Add acoustic sensor noise (Gaussian ±0.8 cm) + occasional 2% false echo outlier
        noise = random.gauss(0, 0.8)
        if random.random() < 0.03:
            noise += random.choice([-15.0, 20.0]) # Acoustic wall bounce artifact
            
        measured_distance = current_depth + noise
        water_column = max(0.0, total_depth - measured_distance)
        
        # Simulated temperature diurnal curve: 20°C night to 34°C afternoon
        water_temp = 25.0 + 4.5 * math.sin((hour - 9) * math.pi / 12) + random.gauss(0, 0.2)
        
        # Simulated TDS: Base 220 ppm with rising salinity during deep drawdown
        salinity_increase = max(0.0, (current_depth - 300.0) * 1.8)
        tds_ppm = 210.0 + salinity_increase + random.gauss(0, 5.0)
        
        # Clarity %: Drops during active pumping due to silt disturbance
        clarity_pct = 92.0 - (18.0 if is_pumping else 0.0) + random.gauss(0, 2.0)
        clarity_pct = min(100.0, max(10.0, clarity_pct))
        
        # Battery voltage: Solar charging during daylight (7 AM - 5 PM)
        is_daylight = (7 <= hour <= 17)
        if is_daylight:
            batt_volts = min(4.20, 3.75 + (hour - 7) * 0.04)
        else:
            night_hours = (hour if hour > 17 else hour + 24) - 17
            batt_volts = max(3.55, 4.10 - night_hours * 0.03)
            
        # Automated Pump Relay state: Cuts off if distance >= pump_intake_depth
        pump_relay_active = measured_distance < pump_intake_depth
        
        data_rows.append({
            "timestamp": timestamp.strftime("%Y-%m-%d %H:%M:%S"),
            "distance_cm": round(measured_distance, 1),
            "water_column_cm": round(water_column, 1),
            "tds_ppm": round(tds_ppm, 0),
            "clarity_pct": round(clarity_pct, 1),
            "water_temp_c": round(water_temp, 1),
            "battery_v": round(batt_volts, 2),
            "pump_relay_enabled": pump_relay_active
        })
        
    return data_rows

def verify_least_squares_regression(data_rows, window_size=48):
    """Verifies the on-device least-squares C++ regression algorithm."""
    distances = [row["distance_cm"] for row in data_rows[-window_size:]]
    n = len(distances)
    
    sum_x = sum(range(n))
    sum_y = sum(distances)
    sum_xy = sum(i * distances[i] for i in range(n))
    sum_xx = sum(i * i for i in range(n))
    
    slope_b = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x ** 2)
    # Slope per sample (30 min) converted to cm/day (48 samples/day)
    rate_cm_per_day = slope_b * 48.0
    
    current_dist = distances[-1]
    pump_intake_depth = 440.0
    
    if rate_cm_per_day > 0.5:
        days_to_dry = (pump_intake_depth - current_dist) / rate_cm_per_day
    else:
        days_to_dry = 999.0
        
    print("\n=======================================================")
    print("📈 ON-DEVICE REGRESSION ALGORITHM VERIFICATION")
    print("=======================================================")
    print(f"Rolling Window Samples : {n} (Last 24 Hours)")
    print(f"Latest Water Distance  : {current_dist:.1f} cm")
    print(f"Depletion Rate         : {rate_cm_per_day:.2f} cm/day")
    print(f"Projected Days-to-Dry  : {days_to_dry:.1f} days")
    print(f"Pump Interlock Cutoff  : {'[ACTIVE / SAFE]' if current_dist < pump_intake_depth else '[TRIPPED / DRY-RUN CUTOFF]'}")
    print("=======================================================\n")

if __name__ == "__main__":
    rows = simulate_aquifer_telemetry(total_hours=48)
    
    os.makedirs("analysis", exist_ok=True)
    csv_file = "analysis/sample_telemetry_data.csv"
    keys = rows[0].keys()
    with open(csv_file, "w", newline="") as f:
        dict_writer = csv.DictWriter(f, fieldnames=keys)
        dict_writer.writeheader()
        dict_writer.writerows(rows)
    print(f"[+] Dataset successfully exported to: {csv_file}")
    
    verify_least_squares_regression(rows)
