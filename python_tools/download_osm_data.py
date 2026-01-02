#!/usr/bin/env python3
"""
Enhanced OpenStreetMap Data Downloader
Downloads NAMED locations (chowks, intersections) from Pakistan cities

This version prioritizes:
1. Named intersections (highway=crossing with name tag)
2. Named traffic signals
3. Named junctions
4. Major landmarks (amenities with names)

OUTPUT: pakistan_osm_junctions_named.json
"""

import requests
import json
import time
from datetime import datetime

print("🗺️  Downloading NAMED Locations from OpenStreetMap...")
print("📡 Using Overpass API (Prioritizing Named Places)\n")

# Pakistan cities with bounding boxes
cities = {
    "Lahore": {
        "bbox": "31.35,74.15,31.65,74.45",
        "target": 500
    },
    "Karachi": {
        "bbox": "24.75,66.85,25.05,67.25",
        "target": 500
    },
    "Islamabad": {
        "bbox": "33.50,72.95,33.75,73.20",
        "target": 300
    },
    "Faisalabad": {
        "bbox": "31.30,72.95,31.50,73.20",
        "target": 200
    },
    "Rawalpindi": {
        "bbox": "33.50,73.00,33.65,73.15",
        "target": 200
    },
    "Multan": {
        "bbox": "30.10,71.40,30.30,71.55",
        "target": 150
    }
}

all_junctions = []
junction_id = 1
seen_names = set()  # Avoid duplicates

def clean_name(name, city):
    """Clean and validate junction name"""
    if not name:
        return None
    
    # Skip generic names
    generic_keywords = ['junction', 'node', 'point', 'unnamed']
    name_lower = name.lower()
    for keyword in generic_keywords:
        if name_lower.startswith(keyword) or name_lower == keyword:
            return None
    
    # Add "Chowk" suffix if it's a major intersection
    if ' ' in name and 'chowk' not in name_lower and 'road' not in name_lower:
        # Check if it might be a chowk name
        major_indicators = ['liberty', 'kalma', 'thokar', 'model', 'garden']
        for indicator in major_indicators:
            if indicator in name_lower:
                name += " Chowk"
                break
    
    return name

for city_name, city_data in cities.items():
    print(f"📍 Processing {city_name}...")
    
    overpass_url = "http://overpass-api.de/api/interpreter"
    
    # Enhanced query - prioritize NAMED elements
    query = f"""
    [bbox:{city_data['bbox']}][out:json][timeout:60];
    (
      // Named traffic signals
      node["highway"="traffic_signals"]["name"];
      
      // Named crossings and junctions
      node["highway"="crossing"]["name"];
      node["highway"="motorway_junction"]["name"];
      
      // Major intersections (ways with names)
      way["highway"="primary"]["name"];
      way["highway"="secondary"]["name"];
      
      // Named landmarks
      node["amenity"]["name"];
      node["place"="square"]["name"];
    );
    out center;
    """
    
    try:
        response = requests.post(overpass_url, data={"data": query}, timeout=120)
        
        if response.status_code == 200:
            data = response.json()
            elements = data.get('elements', [])
            
            city_count = 0
            
            for elem in elements:
                # Get coordinates
                if 'lat' in elem and 'lon' in elem:
                    lat, lon = elem['lat'], elem['lon']
                elif 'center' in elem:
                    lat, lon = elem['center']['lat'], elem['center']['lon']
                else:
                    continue
                
                tags = elem.get('tags', {})
                name = tags.get('name', '')
                
                # Clean and validate name
                clean = clean_name(name, city_name)
                if not clean:
                    continue
                
                # Skip duplicates
                name_key = f"{clean}_{city_name}"
                if name_key in seen_names:
                    continue
                seen_names.add(name_key)
                
                # Extract area/suburb
                area = (tags.get('addr:suburb') or 
                       tags.get('addr:district') or
                       tags.get('place') or
                       'Central')
                
                # Determine if traffic signal
                has_signal = (tags.get('highway') == 'traffic_signals' or
                            'traffic' in tags.get('highway', '').lower())
                
                junction = {
                    "id": junction_id,
                    "name": clean,
                    "latitude": round(lat, 6),
                    "longitude": round(lon, 6),
                    "city": city_name,
                    "area": area,
                    "hasTrafficSignal": has_signal,
                    "osmId": elem.get('id'),
                    "highway_type": tags.get('highway', 'intersection')
                }
                
                all_junctions.append(junction)
                junction_id += 1
                city_count += 1
            
            print(f"   ✅ Found {city_count} NAMED junctions")
            print(f"   Total: {len(all_junctions)}\n")
            
            # Rate limiting
            time.sleep(3)
            
        else:
            print(f"   ❌ HTTP Error {response.status_code}\n")
    
    except Exception as e:
        print(f"   ❌ Error: {str(e)}\n")

# Add some common Lahore landmarks manually if missing
lahore_landmarks = [
    {"id": 9000, "name": "Liberty Chowk", "latitude": 31.5100, "longitude": 74.3406, 
     "city": "Lahore", "area": "Gulberg", "hasTrafficSignal": True},
    {"id": 9001, "name": "Kalma Chowk", "latitude": 31.5204, "longitude": 74.3587,
     "city": "Lahore", "area": "Main Boulevard", "hasTrafficSignal": True},
    {"id": 9002, "name": "Thokar Niaz Baig", "latitude": 31.4343, "longitude": 74.2721,
     "city": "Lahore", "area": "Raiwind Road", "hasTrafficSignal": True},
    {"id": 9003, "name": "Ghazi Chowk", "latitude": 31.4840, "longitude": 74.3861,
     "city": "Lahore", "area": "Samanabad", "hasTrafficSignal": True},
    {"id": 9004, "name": "Bhatti Chowk", "latitude": 31.4156, "longitude": 74.2956,
     "city": "Lahore", "area": "Shahdara", "hasTrafficSignal": True},
    {"id": 9005, "name": "Data Darbar Chowk", "latitude": 31.5784, "longitude": 74.3199,
     "city": "Lahore", "area": "Old City", "hasTrafficSignal": True}
]

# Add missing landmarks
for landmark in lahore_landmarks:
    name_key = f"{landmark['name']}_Lahore"
    if name_key not in seen_names:
        all_junctions.append(landmark)
        junction_id += 1

# Sort by city and name
all_junctions.sort(key=lambda x: (x['city'], x['name']))

# Re-assign sequential IDs
for i, junction in enumerate(all_junctions, start=1):
    junction['id'] = i

# Save to JSON
output = {
    "metadata": {
        "source": "OpenStreetMap via Overpass API (Enhanced)",
        "generated": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "total_junctions": len(all_junctions),
        "cities": list(cities.keys()),
        "description": "Named traffic junctions and landmarks",
        "filter": "Only locations with meaningful names",
        "version": "2.0"
    },
    "junctions": all_junctions
}

output_file = "pakistan_osm_junctions_named.json"

with open(output_file, 'w', encoding='utf-8') as f:
    json.dump(output, f, indent=2, ensure_ascii=False)

print(f"\n✅ SUCCESS!")
print(f"📊 Total Named Junctions: {len(all_junctions)}")
print(f"💾 Saved to: {output_file}")
print(f"📁 File size: {len(json.dumps(output))/1024:.2f} KB")
print(f"\n🔍 Sample names:")
for junction in all_junctions[:10]:
    print(f"   • {junction['name']} ({junction['city']})")






























# import requests
# import json
# import time

# print("🗺️  Downloading Pakistan Traffic Junctions from OpenStreetMap...")
# print("📡 Using Overpass API...\n")

# # Major Pakistan cities with their bounding boxes
# cities = {
#     "Lahore": {
#         "bbox": "31.35,74.15,31.65,74.45",
#         "target": 2000
#     },
#     "Karachi": {
#         "bbox": "24.75,66.85,25.05,67.25",
#         "target": 3000
#     },
#     "Islamabad": {
#         "bbox": "33.50,72.95,33.75,73.20",
#         "target": 1500
#     },
#     "Faisalabad": {
#         "bbox": "31.30,72.95,31.50,73.20",
#         "target": 1000
#     },
#     "Rawalpindi": {
#         "bbox": "33.50,73.00,33.65,73.15",
#         "target": 800
#     },
#     "Multan": {
#         "bbox": "30.10,71.40,30.30,71.55",
#         "target": 700
#     }
# }

# all_junctions = []
# junction_id = 1

# for city_name, city_data in cities.items():
#     print(f"📍 Processing {city_name}...")
    
#     # Overpass API query
#     overpass_url = "http://overpass-api.de/api/interpreter"
    
#     query = f"""
#     [bbox:{city_data['bbox']}][out:json];
#     (
#       node["highway"="traffic_signals"];
#       node["highway"="crossing"];
#       node["highway"="motorway_junction"];
#     );
#     out body;
#     """
    
#     try:
#         response = requests.post(overpass_url, data={"data": query}, timeout=60)
        
#         if response.status_code == 200:
#             data = response.json()
#             nodes = data.get('elements', [])
            
#             for node in nodes:
#                 # Extract junction data
#                 junction = {
#                     "id": junction_id,
#                     "name": node.get('tags', {}).get('name', f'{city_name} Junction {junction_id}'),
#                     "latitude": node['lat'],
#                     "longitude": node['lon'],
#                     "city": city_name,
#                     "area": node.get('tags', {}).get('addr:suburb', 
#                            node.get('tags', {}).get('place', 'Central')),
#                     "hasTrafficSignal": node.get('tags', {}).get('highway') == 'traffic_signals',
#                     "osmId": node['id']
#                 }
                
#                 all_junctions.append(junction)
#                 junction_id += 1
            
#             print(f"   ✅ Found {len(nodes)} junctions in {city_name}")
#             print(f"   Total so far: {len(all_junctions)}\n")
            
#             # Be nice to Overpass API - wait between requests
#             time.sleep(2)
            
#         else:
#             print(f"   ❌ Error downloading {city_name} data: {response.status_code}\n")
    
#     except Exception as e:
#         print(f"   ❌ Error: {str(e)}\n")

# # Save to JSON file
# output = {
#     "metadata": {
#         "source": "OpenStreetMap via Overpass API",
#         "generated": time.strftime("%Y-%m-%d %H:%M:%S"),
#         "total_junctions": len(all_junctions),
#         "cities": list(cities.keys()),
#         "description": "Real traffic junctions from OpenStreetMap"
#     },
#     "junctions": all_junctions
# }

# output_file = "../data/pakistan_osm_junctions.json"

# with open(output_file, 'w', encoding='utf-8') as f:
#     json.dump(output, f, indent=2, ensure_ascii=False)

# print(f"\n✅ SUCCESS!")
# print(f"📊 Total Junctions Downloaded: {len(all_junctions)}")
# print(f"💾 Saved to: {output_file}")
# print(f"📁 File size: {len(json.dumps(output))/1024:.2f} KB")

