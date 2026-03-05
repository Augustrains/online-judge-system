from flask import Flask, request, jsonify, render_template
import requests
import mysql.connector
from datetime import datetime

app = Flask(__name__, static_folder='.', static_url_path='')

# 心知天气API配置
API_KEY = "MJgPiYpEd1ndzXJQZUVZTH9BZ7inlw4o"  # 替换为你的API Key
API_URL = "http://gfapi.mlogcn.com/weather/v001/now"

# 数据库配置
DB_CONFIG = {
    "host": "8.140.17.222",
    "user": "weather",
    "password": "x36tfGjDaMfcHHkw",
    "database": "weather",
    "port": 3306,
    "autocommit": True
}

# 增强版的城市名称到地区代码的映射表
CITY_CODE_MAPPING = {
    # 主要城市中英文映射
    "北京": "101010100", "Beijing": "101010100",
    "上海": "101020100", "Shanghai": "101020100",
    "天津": "101030100", "Tianjin": "101030100",
    "重庆": "101040100", "Chongqing": "101040100",
    "哈尔滨": "101050101", "Harbin": "101050101",
    "长春": "101060101", "Changchun": "101060101",
    "沈阳": "101070101", "Shenyang": "101070101",
    "石家庄": "101080101", "Shijiazhuang": "101080101",
    "太原": "101090101", "Taiyuan": "101090101",
    "西安": "101100101", "Xi'an": "101100101",
    "济南": "101110101", "Jinan": "101110101",
    "青岛": "101120101", "Qingdao": "101120101",
    "南京": "101130101", "Nanjing": "101130101",
    "杭州": "101140101", "Hangzhou": "101140101",
    "宁波": "101150101", "Ningbo": "101150101",
    "合肥": "101160101", "Hefei": "101160101",
    "福州": "101170101", "Fuzhou": "101170101",
    "厦门": "101180101", "Xiamen": "101180101",
    "南昌": "101190101", "Nanchang": "101190101",
    "长沙": "101200101", "Changsha": "101200101",
    "武汉": "101210101", "Wuhan": "101210101",
    "广州": "101220101", "Guangzhou": "101220101",
    "深圳": "101230101", "Shenzhen": "101230101",
    "珠海": "101240101", "Zhuhai": "101240101",
    "南宁": "101250101", "Nanning": "101250101",
    "成都": "101260101", "Chengdu": "101260101",
    "贵阳": "101270101", "Guiyang": "101270101",
    "昆明": "101280101", "Kunming": "101280101",
    "拉萨": "101290101", "Lhasa": "101290101",
    "西安": "101300101", "Xi'an": "101300101",

    # 增加更多城市别名和常见变体
    "广州市": "101220101", "深圳市": "101230101",
    "北京市": "101010100", "上海市": "101020100",
    "天津市": "101030100", "重庆市": "101040100",
    "哈尔滨市": "101050101", "长春市": "101060101",
    "沈阳市": "101070101", "石家庄市": "101080101",
    "太原市": "101090101", "西安市": "101100101",
    "济南市": "101110101", "青岛市": "101120101",
    "南京市": "101130101", "杭州市": "101140101",
    "宁波市": "101150101", "合肥市": "101160101",
    "福州市": "101170101", "厦门市": "101180101",
    "南昌市": "101190101", "长沙市": "101200101",
    "武汉市": "101210101", "广州市": "101220101",
    "深圳市": "101230101", "珠海市": "101240101",
    "南宁市": "101250101", "成都市": "101260101",
    "贵阳市": "101270101", "昆明市": "101280101",
    "拉萨市": "101290101",
}

# 使用多个IP定位服务进行地理位置查询
def get_location_by_ip(client_ip):
    # 备选定位服务列表
    location_services = [
        {
            "name": "ip-api",
            "url": f"http://ip-api.com/json/{client_ip}?fields=status,city,country,countryCode,regionName",
            "parser": lambda data: {
                "city": data.get("city"),
                "country": data.get("country"),
                "region": data.get("regionName"),
                "country_code": data.get("countryCode")
            }
        },
        {
            "name": "ipinfo",
            "url": f"https://ipinfo.io/{client_ip}/json",
            "parser": lambda data: {
                "city": data.get("city"),
                "country": data.get("country"),
                "region": data.get("region"),
                "country_code": data.get("country")
            }
        },
        {
            "name": "ipgeolocation",
            "url": f"https://api.ipgeolocation.io/ipgeo?apiKey=YOUR_IPGEOLOCATION_KEY&ip={client_ip}",
            "parser": lambda data: {
                "city": data.get("city"),
                "country": data.get("country_name"),
                "region": data.get("state_prov"),
                "country_code": data.get("country_code2")
            }
        }
    ]

    # 依次尝试每个定位服务
    for service in location_services:
        try:
            print(f"尝试使用 {service['name']} 定位 IP: {client_ip}")
            response = requests.get(service["url"], timeout=5)

            if response.status_code == 200:
                data = response.json()

                # 检查响应状态（不同服务状态字段不同）
                if service["name"] == "ip-api" and data.get("status") != "success":
                    print(f"{service['name']} 定位失败: {data.get('message', '未知错误')}")
                    continue

                # 解析定位结果
                location = service["parser"](data)

                # 验证定位结果的有效性
                if location["city"]:
                    print(f"{service['name']} 定位成功: {location['city']}, {location['country']}")
                    return location
                else:
                    print(f"{service['name']} 返回的城市信息为空")
            else:
                print(f"{service['name']} 请求失败，状态码: {response.status_code}")

        except Exception as e:
            print(f"使用 {service['name']} 定位时出错: {str(e)}")

    # 如果所有服务都失败
    print(f"所有定位服务都无法获取 {client_ip} 的位置信息")
    return None

# 改进的城市名称到地区代码映射函数
def map_city_to_code(city_name, country_code=None):
    # 直接从映射表中查找
    area_code = CITY_CODE_MAPPING.get(city_name)
    if area_code:
        return area_code

    # 尝试一些常见的转换和清理
    cleaned_city = city_name.strip().replace("市", "").replace("县", "")
    area_code = CITY_CODE_MAPPING.get(cleaned_city)
    if area_code:
        return area_code

    # 如果是中国地区，可以尝试拼音转换（需要安装pypinyin库）
    try:
        from pypinyin import pinyin, NORMAL
        pinyin_name = ''.join([word[0] for word in pinyin(city_name, style=NORMAL)])
        pinyin_name = pinyin_name.capitalize()  # 首字母大写
        area_code = CITY_CODE_MAPPING.get(pinyin_name)
        if area_code:
            return area_code
    except ImportError:
        print("pypinyin库未安装，跳过拼音转换")

    # 如果有国家代码，可以根据国家提供默认值
    if country_code == "CN":
        print(f"无法映射中国城市: {city_name}，使用默认值北京")
        return "101010100"  # 中国默认返回北京

    print(f"无法映射城市: {city_name}，国家: {country_code}")
    return None

# 将GPS坐标转换为地区代码
def convert_gps_to_areacode(latitude, longitude):
    try:
        # 调用心知天气逆地理编码API
        url = f"http://gfapi.mlogcn.com/function/v001/poi?lonlat={longitude},{latitude}&language=CHN&withTz=false&withPoi=false&key={API_KEY}&output_type=json"
        response = requests.get(url)
        
        if response.status_code == 200:
            data = response.json()
            if data.get("status") == 0 and "location" in data:
                area_code = data["location"]["areacode"]
                location_name = data["location"]["name"]
                return area_code, location_name
            else:
                print(f"心知天气逆地理编码API返回错误: {data}")
                return None, None
        else:
            print(f"心知天气逆地理编码API请求失败，状态码: {response.status_code}")
            return None, None
            
    except Exception as e:
        print(f"GPS坐标转换为地区代码时出错: {str(e)}")
        return None, None

# 存储天气信息到数据库
def save_weather_to_db(weather_data):
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()

        insert_query = """
        INSERT INTO tweather (area_code, location_name, weather, temp, wind, is_current_location, record_time)
        VALUES (%s, %s, %s, %s, %s, %s, %s)
        """
        
        for item in weather_data:
            area_code = item.get('area_code')
            # 优先使用 weather_data 中的 location_name，否则通过映射表获取
            location_name = item.get('location_name')
            if not location_name and area_code:
                location_name = next((k for k, v in CITY_CODE_MAPPING.items() if v == area_code), '未知')
            
            weather = item.get('weather', '未知')
            temp = item.get('temp', '未知')
            wind = item.get('wind', '未知')
            is_current_location = item.get('is_current_location', False)
            record_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

            cursor.execute(insert_query, (
                area_code, 
                location_name, 
                weather, 
                temp, 
                wind, 
                is_current_location,
                record_time
            ))

        conn.commit()
        cursor.close()
        conn.close()
    except mysql.connector.Error as err:
        print(f"数据库错误: {err}")
    except Exception as e:
        print(f"保存天气数据时出错: {e}")

# 从数据库获取历史天气记录
def get_historical_weather_from_db():
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor(dictionary=True)

        select_query = """
        SELECT area_code, location_name, weather, temp, wind, record_time
        FROM tweather
        ORDER BY record_time DESC
        """
        cursor.execute(select_query)
        result = cursor.fetchall()

        cursor.close()
        conn.close()

        return result
    except mysql.connector.Error as err:
        print(f"数据库错误: {err}")
        return []
    except Exception as e:
        print(f"获取历史天气数据时出错: {e}")
        return []

@app.route('/api/3hour_forecast', methods=['GET'])
def get_3hour_forecast():
    area_code = request.args.get('area_code')
    hours = request.args.get('hours', 24)  # 默认获取24小时预报
    
    if not area_code:
        return jsonify({"error": "需要提供area_code参数"}), 400
    
    try:
        # 构建API请求参数
        params = {
            'areacode': area_code,
            'hours': hours,
            'key': API_KEY
        }

        # 发送请求到心知天气3小时预报API
        response = requests.get("http://gfapi.mlogcn.com/weather/v001/3hours", params=params)

        # 检查响应状态
        if response.status_code == 200:
            data = response.json()
            if data.get("status") == 0 and "result" in data:
                return jsonify(data["result"])
            else:
                print(f"心知天气3小时预报API返回错误，完整数据: {data}")
                return jsonify({"error": "API返回错误", "details": data}), 500
        else:
            print(f"心知天气3小时预报API请求失败，状态码: {response.status_code}")
            return jsonify({"error": f"API请求失败，状态码: {response.status_code}"}), 500

    except Exception as e:
        print(f"获取3小时预报数据时出错: {str(e)}")
        return jsonify({"error": f"获取3小时预报数据时出错: {str(e)}"}), 500

@app.route('/')
def index():
    return app.send_static_file('weather.html')

@app.route('/api/weather', methods=['GET'])
def get_weather():
    results = []
    client_ip = request.remote_addr

    try:
        # 使用增强的IP定位函数
        location = get_location_by_ip(client_ip)

        if location and location["city"]:
            # 尝试从映射表中获取地区代码
            area_code = map_city_to_code(location["city"], location["country_code"])

            if area_code:
                # 找到了匹配的地区代码
                weather_data = fetch_weather(area_code)
                weather_data['is_current_location'] = True
                weather_data['location_name'] = location["city"]
                results.append(weather_data)
            else:
                # 没有找到匹配的地区代码
                print(f"未找到城市 '{location['city']}' 对应的地区代码，使用长春的位置")
                location = {
                    "city": "长春",
                    "country": "中国",
                    "region": "吉林",
                    "country_code": "CN"
                }
                area_code = map_city_to_code(location["city"], location["country_code"])
                weather_data = fetch_weather(area_code)
                weather_data['is_current_location'] = True
                weather_data['location_name'] = location["city"]
                results.append(weather_data)
        else:
            # 无法获取位置信息，使用长春的位置
            print(f"无法获取 {client_ip} 的位置信息，使用长春的位置")
            location = {
                "city": "长春",
                "country": "中国",
                "region": "吉林",
                "country_code": "CN"
            }
            area_code = map_city_to_code(location["city"], location["country_code"])
            weather_data = fetch_weather(area_code)
            weather_data['is_current_location'] = True
            weather_data['location_name'] = location["city"]
            results.append(weather_data)

    except Exception as e:
        # 发生异常时，也使用长春的位置
        print(f"IP定位过程出错: {str(e)}，使用长春的位置")
        location = {
            "city": "长春",
            "country": "中国",
            "region": "吉林",
            "country_code": "CN"
        }
        area_code = map_city_to_code(location["city"], location["country_code"])
        weather_data = fetch_weather(area_code)
        weather_data['is_current_location'] = True
        weather_data['location_name'] = location["city"]
        results.append(weather_data)

    # 获取前端传来的地区代码列表，获取其他指定地区的天气
    area_codes = request.args.get('area_codes', '')
    if area_codes:
        area_codes = area_codes.split(',')
        for area_code in area_codes:
            weather_data = fetch_weather(area_code)
            # 确保从下拉框选择的地区有正确的名称
            if area_code in CITY_CODE_MAPPING.values():
                location_name = next((k for k, v in CITY_CODE_MAPPING.items() if v == area_code), '未知')
                weather_data['location_name'] = location_name
            results.append(weather_data)

    # 保存天气数据到数据库
    save_weather_to_db(results)

    return jsonify(results)

# 通过GPS坐标获取天气
@app.route('/api/gps_weather', methods=['POST'])
def get_weather_by_gps():
    try:
        # 获取前端传来的GPS坐标
        data = request.get_json()
        latitude = data.get('latitude')
        longitude = data.get('longitude')
        
        if not latitude or not longitude:
            return jsonify({"error": "需要提供latitude和longitude参数"}), 400
        
        # 将GPS坐标转换为地区代码
        area_code, location_name = convert_gps_to_areacode(latitude, longitude)
        
        if not area_code:
            return jsonify({"error": "无法将GPS坐标转换为地区代码"}), 400
        
        # 获取天气数据
        weather_data = fetch_weather(area_code)
        weather_data['is_current_location'] = True
        weather_data['location_name'] = location_name
        
        # 获取3小时预报数据
        forecast_data = get_3hour_forecast_data(area_code)
        
        # 保存天气数据到数据库
        save_weather_to_db([weather_data])
        
        return jsonify([weather_data])
        
    except Exception as e:
        print(f"通过GPS获取天气数据时出错: {str(e)}")
        return jsonify({"error": f"通过GPS获取天气数据时出错: {str(e)}"}), 500

# 获取3小时预报数据
def get_3hour_forecast_data(area_code):
    try:
        # 构建API请求参数
        params = {
            'areacode': area_code,
            'hours': 24,  # 获取24小时预报
            'key': API_KEY
        }

        # 发送请求到心知天气3小时预报API
        response = requests.get("http://gfapi.mlogcn.com/weather/v001/3hours", params=params)

        # 检查响应状态
        if response.status_code == 200:
            data = response.json()
            if data.get("status") == 0 and "result" in data:
                return data["result"]
            else:
                print(f"心知天气3小时预报API返回错误，完整数据: {data}")
                return None
        else:
            print(f"心知天气3小时预报API请求失败，状态码: {response.status_code}")
            return None

    except Exception as e:
        print(f"获取3小时预报数据时出错: {str(e)}")
        return None

# 获取历史天气记录
@app.route('/api/history', methods=['GET'])
def get_history():
    history = get_historical_weather_from_db()
    return jsonify(history)

# 获取天气数据
def fetch_weather(area_code):
    try:
        # 构建API请求参数
        params = {
            'areacode': area_code,
            'key': API_KEY
        }

        # 发送请求到心知天气API
        response = requests.get(API_URL, params=params)

        # 检查响应状态
        if response.status_code == 200:
            data = response.json()
            # 更详细地检查API返回数据结构
            if 'result' in data and 'realtime' in data['result']:
                realtime = data['result']['realtime']
                return {
                    'area_code': area_code,
                    'weather': realtime['text'],
                    'temp': realtime['temp'],
                    'wind': f"{realtime['wind_dir']} {realtime['wind_class']}级"
                }
            else:
                print(f"心知天气API返回格式不正确，完整数据: {data}")
                return {
                    'area_code': area_code,
                    'error': 'API返回格式不正确',
                    'location_name': '未知'
                }
        else:
            print(f"心知天气API请求失败，状态码: {response.status_code}")
            return {
                'area_code': area_code,
                'error': f'API请求失败，状态码: {response.status_code}',
                'location_name': '未知'
            }

    except Exception as e:
        print(f"获取天气数据时出错: {str(e)}")
        return {
            'area_code': area_code,
            'error': f'获取天气数据时出错: {str(e)}',
            'location_name': '未知'
        }

if __name__ == '__main__':
    # 确保在长春网络环境下运行时，以下IP地址为长春本地IP（如果需要指定IP运行）
    # 这里可以根据实际情况修改host和port
    app.run(debug=True, port=5001)