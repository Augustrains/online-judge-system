import _thread as thread
import base64
import datetime
import hashlib
import hmac
import json
from urllib.parse import urlparse
import ssl
from datetime import datetime
from time import mktime
from urllib.parse import urlencode
from wsgiref.handlers import format_date_time
from flask import Flask, render_template_string
from flask_cors import CORS
import websocket  # 使用websocket_client
import os  # 用于文件操作和目录检查

answer = ""
content = ''
isFirstcontent = False
app = Flask(__name__)
CORS(app)  # 启用 CORS

# 定义输入文件路径
INPUT_FILE = "/root/OnlineJudge/oj_server/what.txt"
# 定义输出文件路径
FILE_PATH = "/root/OnlineJudge/oj_server/answer.txt"
os.makedirs(os.path.dirname(FILE_PATH), exist_ok=True)  # 确保输出目录存在


class Ws_Param(object):
    # 初始化（不变）
    def __init__(self, APPID, APIKey, APISecret, Spark_url):
        self.APPID = APPID
        self.APIKey = APIKey
        self.APISecret = APISecret
        self.host = urlparse(Spark_url).netloc
        self.path = urlparse(Spark_url).path
        self.Spark_url = Spark_url

    # 生成url（已修正字符串引号问题）
    def create_url(self):
        now = datetime.now()
        date = format_date_time(mktime(now.timetuple()))
        signature_origin = f"host: {self.host}\n" f"date: {date}\n" "GET " f"{self.path} HTTP/1.1"
        signature_sha = hmac.new(
            self.APISecret.encode('utf-8'),
            signature_origin.encode('utf-8'),
            digestmod=hashlib.sha256
        ).digest()
        signature_sha_base64 = base64.b64encode(signature_sha).decode('utf-8')
        authorization_origin = (
            f'api_key="{self.APIKey}", algorithm="hmac-sha256", '
            f'headers="host date request-line", signature="{signature_sha_base64}"'
        )
        authorization = base64.b64encode(authorization_origin.encode('utf-8')).decode('utf-8')
        v = {
            "authorization": authorization,
            "date": date,
            "host": self.host
        }
        return f"{self.Spark_url}?{urlencode(v)}"


# 收到websocket错误的处理（不变）
def on_error(ws, error):
    print("### error:", error)


# 收到websocket关闭的处理（不变）
def on_close(ws, one, two):
    print("WebSocket 连接已关闭")


# 收到websocket连接建立的处理（不变）
def on_open(ws):
    thread.start_new_thread(run, (ws,))


def run(ws, *args):
    data = json.dumps(gen_params(
        appid=ws.appid,
        domain=ws.domain,
        question=ws.question
    ))
    ws.send(data)


# 收到websocket消息的处理（不变）
def on_message(ws, message):
    global answer, content, isFirstcontent
    data = json.loads(message)
    code = data['header']['code']
    if code != 0:
        print(f'请求错误: {code}, {data}')
        ws.close()
        return
    
    choices = data["payload"]["choices"]
    status = choices["status"]
    text = choices['text'][0]
    
    if 'reasoning_content' in text and text['reasoning_content']:
        reasoning_content = text["reasoning_content"]
       # print(reasoning_content, end="")
        isFirstcontent = True
    
    if 'content' in text and text['content']:
        content = text["content"]
        if isFirstcontent:
            print("\n*******************以上为思维链内容，模型回复内容如下********************\n")
       # print(content, end="")
        with open(FILE_PATH, "a", encoding="utf-8") as f:
            f.write(content)  # 添加换行符
        isFirstcontent = False
    
    answer += content
    if status == 2:
        ws.close()


def gen_params(appid, domain, question):
    """生成请求参数"""
    return {
        "header": {
            "app_id": appid,
            "uid": "1234"
        },
        "parameter": {
            "chat": {
                "domain": domain,
                "temperature": 1.2,
                "max_tokens": 32768
            }
        },
        "payload": {
            "message": {
                "text": question
            }
        }
    }


# 新增：将 main 函数定义移到调用之前
def main(appid, api_key, api_secret, Spark_url, domain, question):
    wsParam = Ws_Param(appid, api_key, api_secret, Spark_url)
    websocket.enableTrace(False)
    wsUrl = wsParam.create_url()
    ws = websocket.WebSocketApp(
        wsUrl,
        on_message=on_message,
        on_error=on_error,
        on_close=on_close,
        on_open=on_open
    )
    ws.appid = appid
    ws.question = question
    ws.domain = domain
    ws.run_forever(sslopt={"cert_reqs": ssl.CERT_NONE})


text = []

# 管理对话历史（不变）
def getText(role, content):
    jsoncon = {"role": role, "content": content}
    text.append(jsoncon)
    return text

def getlength(text):
    return sum(len(item["content"]) for item in text)

def checklen(text):
    while getlength(text) > 8000:
        del text[0]
    return text


if __name__ == '__main__':
    # 配置信息
    appid = "7606551a"
    api_secret = "OTY0Y2E1ZDc5NDhiNGZjNGUyYjExYjY1"
    api_key = "ee22d580bf5278a6ad427e35e33bbcf4"
    domain = "x1"
    Spark_url = "wss://spark-api.xf-yun.com/v1/x1"

    # 读取输入文件
    try:
        with open(INPUT_FILE, "r", encoding="utf-8") as f:
            Input = f.read().strip()
    except FileNotFoundError:
        print(f"错误：文件 {INPUT_FILE} 未找到")
        exit(1)
    except Exception as e:
        print(f"读取文件失败：{e}")
        exit(1)

    # 处理输入
    question = checklen(getText("user", Input))
    #print("我:", Input)
   # print("星火:", end="")
    answer = ""
    
    # 调用 main 函数（此时函数已定义）
    main(appid, api_key, api_secret, Spark_url, domain, question)
    
    getText("assistant", answer)

