# coding: utf-8
import os
import sys
import subprocess

# 检查并自动安装缺失的依赖
def install_missing_packages():
    try:
        import websocket
    except ImportError:
        print("正在安装缺失的依赖: websocket-client")
        subprocess.check_call([sys.executable, "-m", "pip", "install", "websocket-client"])
        print("websocket-client 安装成功")

# 安装缺失的依赖
install_missing_packages()

import _thread as thread
import base64
import hashlib
import hmac
import json
import ssl
from datetime import datetime
from time import mktime
from urllib.parse import urlencode
from urllib.parse import urlparse
from wsgiref.handlers import format_date_time
import websocket

class Ws_Param(object):
    # 初始化
    def __init__(self, APIKey, APISecret, gpt_url):
        self.APIKey = APIKey
        self.APISecret = APISecret
        self.host = urlparse(gpt_url).netloc
        self.path = urlparse(gpt_url).path
        self.gpt_url = gpt_url

    # 生成url
    def create_url(self):
        # 生成RFC1123格式的时间戳
        now = datetime.now()
        date = format_date_time(mktime(now.timetuple()))

        # 拼接字符串
        signature_origin = "host: " + self.host + "\n"
        signature_origin += "date: " + date + "\n"
        signature_origin += "GET " + self.path + " HTTP/1.1"

        # 进行hmac-sha256进行加密
        signature_sha = hmac.new(self.APISecret.encode('utf-8'), signature_origin.encode('utf-8'),
                              digestmod=hashlib.sha256).digest()

        signature_sha_base64 = base64.b64encode(signature_sha).decode(encoding='utf-8')

        authorization_origin = f'api_key="{self.APIKey}", algorithm="hmac-sha256", headers="host date request-line", signature="{signature_sha_base64}"'

        authorization = base64.b64encode(authorization_origin.encode('utf-8')).decode(encoding='utf-8')

        # 将请求的鉴权参数组合为字典
        v = {
            "authorization": authorization,
            "date": date,
            "host": self.host
        }
        # 拼接鉴权参数，生成url
        url = self.gpt_url + '?' + urlencode(v)
        # 此处打印出建立连接时候的url,参考本demo的时候可取消上方打印的注释，比对相同参数时生成的url与自己代码生成的url是否一致
        return url


# 收到websocket错误的处理
def on_error(ws, error):
    print("### error:", error)


# 收到websocket关闭的处理
def on_close(ws, close_status_code, close_msg):
    print("### closed ###")


# 收到websocket连接建立的处理
def on_open(ws):
    thread.start_new_thread(run, (ws,))


def run(ws, *args):
    data = json.dumps(parameter)
    # 发送请求参数
    ws.send(data)


# 收到websocket消息的处理
def on_message(ws, message):
    data = json.loads(message)
    print(data)
    code = data['header']['code']
    choices = data["payload"]["choices"]
    status = choices["status"]
    if code != 0:
        print(f'请求错误: {code}, {data}')
        ws.close()
    if status == 2:
        print("#### 关闭会话")
        ws.close()

# webSocket 请求的参数
parameter = {
    "payload": {
        "message": {
            "text": [
                {
                    "role": "system",
                    "content": "你是上海半醒科技有限公司开发的人形机器人，名字叫做“精灵”，为客户提供服务"
                },
                {
                    "role": "user",
                    "content": "你是谁"
                }
            ]
        }
    },
    "parameter": {
        "chat": {
            "max_tokens": 500,
            "domain": "generalv3.5",
            "top_k": 4,
            "temperature": 0.7
        }
    },
    "header": {
        "app_id": "69a4ef4e"
    }
}


def main(api_secret, api_key, gpt_url):
    wsParam = Ws_Param(api_key, api_secret, gpt_url)
    websocket.enableTrace(False)
    wsUrl = wsParam.create_url()
    ws = websocket.WebSocketApp(wsUrl, on_message=on_message, on_error=on_error, on_close=on_close, on_open=on_open)
    ws.run_forever(sslopt={"cert_reqs": ssl.CERT_NONE})


if __name__ == "__main__":
    main(
        api_secret='NDMxYzMzMTI3MzU4NDc5YTdmMDFkNWFm',
        api_key='026e8878757790d1f0a537ef3985aa0f',
        gpt_url='wss://spark-api.xf-yun.com/v3.5/chat',  # 例如 wss://spark-api.xf-yun.com/v4.0/chat
    )
