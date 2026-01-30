#include "CCosmosHostApi.h"
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <thread>

/*
	Cosmos宿主与组件引擎通讯时，字符串类型的编码都为UTF8编码，需要宿主开发人员注意
*/
static std::atomic<bool> g_stop{false};

static void on_sigint(int) {
	g_stop.store(true, std::memory_order_relaxed);
}

int main(int argc, char* argv[])
{
	std::signal(SIGINT, on_sigint);

	//向Cosmos引擎注册通知回调函数，等待Cosmos引擎初始化完成后加载组件
	CCosmosApi::GetInstance()->RegistNotify([](const std::string& strNotify) {
		
		rapidjson::Document docNotify;
		docNotify.Parse(strNotify.c_str());

		if (!docNotify.HasParseError() && docNotify.HasMember("ActionContext") && docNotify["ActionContext"].HasMember("NotifyType"))
		{
			//通知类型为VendorStartSuccessfully的即为Cosmos引擎初始化完成，可以加载组件
			if (std::string(docNotify["ActionContext"]["NotifyType"].GetString()) == "VendorStartSuccessfully")
			{
				std::string strResult;

				/*------------------------------*/
				/*        创建组件方法			*/
				/*     创建一个组件demo组件		*/
				/*------------------------------*/
				{
					rapidjson::Document doc;
					doc.SetObject();
					rapidjson::Value jsPreference(rapidjson::kObjectType), jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);
					jsPreference.AddMember(rapidjson::StringRef("ParentHandle"), rapidjson::StringRef("0"), doc.GetAllocator());
					jsPreference.AddMember(rapidjson::StringRef("TitleBarVisibility"), rapidjson::StringRef("Visible"), doc.GetAllocator());
					jsPreference.AddMember(rapidjson::StringRef("WindowVisibility"), rapidjson::StringRef("Visible"), doc.GetAllocator());
					jsPreference.AddMember(rapidjson::StringRef("ResizeMode"), rapidjson::StringRef("CanResize"), doc.GetAllocator());
					jsPreference.AddMember(rapidjson::StringRef("WidgetWidth"), 600, doc.GetAllocator());
					jsPreference.AddMember(rapidjson::StringRef("WidgetHeight"), 600, doc.GetAllocator());
					jsPreference.AddMember(rapidjson::StringRef("BorderThickness"), rapidjson::StringRef("1"), doc.GetAllocator());
					jsPreference.AddMember(rapidjson::StringRef("WindowTop"), 0, doc.GetAllocator());
					jsPreference.AddMember(rapidjson::StringRef("WindowLeft"), 0, doc.GetAllocator());

					jsParameters.AddMember(rapidjson::StringRef("WidgetPreference"), jsPreference, doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("WidgetGuid"), rapidjson::StringRef("b0fd068e-2021-4619-acc0-53cda8d94a37"), doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("AppGuid"), rapidjson::StringRef("1F74493F-E84D-4193-8FCE-F7CF4116EA63"), doc.GetAllocator());

					jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());						//被调用对象参数列表
					jsActionContext.AddMember(rapidjson::StringRef("Invoker"), rapidjson::StringRef("00000000"), doc.GetAllocator());		//被调用对象、00000000代表向cosmos发起请求，其他则为组件示例id
					jsActionContext.AddMember(rapidjson::StringRef("Function"), rapidjson::StringRef("CreateWidget"), doc.GetAllocator());	//被调用方法

					doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());								//请求内容
					doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());						//请求类型
					doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());			//当前调用者实例名

					rapidjson::StringBuffer strBuf;
					rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
					doc.Accept(writer);

					std::string data = strBuf.GetString();
					strResult = CCosmosApi::GetInstance()->Invoke("CreateWidget", data);
				}
				
				/*------------------------------*/
				/*        创建页面方法			*/
				/*		  创建一个页面		    */
				/*------------------------------*/
				{
					rapidjson::Document doc;
					doc.SetObject();
					rapidjson::Value jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);
					jsParameters.AddMember(rapidjson::StringRef("ParentHandle"), rapidjson::StringRef("0"), doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("TitleBarVisibility"), rapidjson::StringRef("Visible"), doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("WindowVisibility"), rapidjson::StringRef("Visible"), doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("ResizeMode"), rapidjson::StringRef("CanResize"), doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("WidgetWidth"), 600, doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("WidgetHeight"), 600, doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("BorderThickness"), rapidjson::StringRef("1"), doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("WindowTop"), 0, doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("WindowLeft"), 0, doc.GetAllocator());
					jsParameters.AddMember(rapidjson::StringRef("XmlPath"), rapidjson::StringRef("D:\\git\\itrader\\Cosmos_yinhe\\Source\\文档\\宿主接入\\CosmosHost\\Cosmos\\WndManager\\a25320f9-815e-4859-9f15-2cb8b1f598bc_1.config"), doc.GetAllocator());

					jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());						//被调用对象参数列表
					jsActionContext.AddMember(rapidjson::StringRef("Invoker"), rapidjson::StringRef("00000000"), doc.GetAllocator());		//被调用对象、00000000代表向cosmos发起请求，其他则为组件示例id
					jsActionContext.AddMember(rapidjson::StringRef("Function"), rapidjson::StringRef("CreatePage"), doc.GetAllocator());	//被调用方法

					doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());								//请求内容
					doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());						//请求类型
					doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());			//当前调用者实例名

					rapidjson::StringBuffer strBuf;
					rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
					doc.Accept(writer);

					std::string data = strBuf.GetString();
					strResult = CCosmosApi::GetInstance()->Invoke("CreatePage", data);

				}
				rapidjson::Document docResult;
				docResult.Parse(strResult.c_str());
				std::string strWidgetHandle;	//保存组件demo组件句柄
				std::string strWindowsHandle;	//保存组件demo窗口句柄,可以通过该句柄调用setparent方法内嵌到需要嵌入的窗口上
				if (!docResult.HasParseError())
				{
					auto nCode = docResult["Code"].GetInt64();
					printf("code:%d\n", nCode);
					//调用成功
					if (nCode == 200)
					{
						rapidjson::Document docTmp;
						docTmp.Parse(docResult["Data"].GetString());
						if (!docTmp.HasParseError())
						{
							strWidgetHandle = docTmp["ActionContext"]["Return"]["WidgetHandle"].GetString();
							strWindowsHandle = docTmp["ActionContext"]["Return"]["WindowHandle"].GetString();
						}
						
					}
				}

				if (!strWidgetHandle.empty())
				{
					/*------------------------------*/
					/*         像组件发起请求		    */
					/*      调用组件GetName方法	    */
					/*------------------------------*/
					{
						rapidjson::Document doc;
						doc.SetObject();
						rapidjson::Value jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);

						jsParameters.AddMember(rapidjson::StringRef("Name"), rapidjson::StringRef("Cosmos"), doc.GetAllocator());

						jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
						jsActionContext.AddMember(rapidjson::StringRef("Function"), rapidjson::StringRef("GetName"), doc.GetAllocator());
						jsActionContext.AddMember(rapidjson::StringRef("Invoker"), rapidjson::StringRef(strWidgetHandle.c_str()), doc.GetAllocator());

						doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
						doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());
						doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

						rapidjson::StringBuffer strBuf;
						rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
						doc.Accept(writer);

						std::string data = strBuf.GetString();
						strResult = CCosmosApi::GetInstance()->Invoke("GetName", data);

						rapidjson::Document docResult;
						docResult.Parse(strResult.c_str());
						std::string strWidgetHandle;	//保存组件demo组件句柄
						std::string strWindowsHandle;	//保存组件demo窗口句柄,可以通过该句柄调用setparent方法内嵌到需要嵌入的窗口上
						if (!docResult.HasParseError())
						{
							auto nCode = docResult["Code"].GetInt64();
							//调用成功
							if (nCode == 200)
							{
								rapidjson::Document docTmp;
								docTmp.Parse(docResult["Data"].GetString());
								if (!docTmp.HasParseError())
								{
									std::string result = docTmp["ActionContext"]["Return"]["Result"].GetString();
									printf("recv com getname result:%s\n", result.c_str());
								}
							}
						}
					}

					/*------------------------------*/
					/*        像组件发起通知		    */
					/*     像组件发送setname通知		*/
					/*------------------------------*/
					{
						rapidjson::Document doc;
						doc.SetObject();
						rapidjson::Value jsPreference(rapidjson::kObjectType), jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);

						jsParameters.AddMember(rapidjson::StringRef("Name"), "Cosmos", doc.GetAllocator());

						jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
						jsActionContext.AddMember(rapidjson::StringRef("NotifyType"), rapidjson::StringRef("SetName"), doc.GetAllocator());
						jsActionContext.AddMember(rapidjson::StringRef("Notifier"), rapidjson::StringRef(strWidgetHandle.c_str()), doc.GetAllocator());

						doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
						doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Notify"), doc.GetAllocator());
						doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

						rapidjson::StringBuffer strBuf;
						rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
						doc.Accept(writer);

						std::string data = strBuf.GetString();
						CCosmosApi::GetInstance()->Notify("SetName", data);
					}


					/*------------------------------*/
					/*         更新组件属性		    */
					/*     更新组件demo组件属性		*/
					/*------------------------------*/
					{
						std::this_thread::sleep_for(std::chrono::seconds(10));				//sleep几秒钟，看下移动效果
						rapidjson::Document doc;
						doc.SetObject();
						rapidjson::Value jsPreference(rapidjson::kObjectType), jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);

						jsParameters.AddMember(rapidjson::StringRef("WidgetHandle"), rapidjson::StringRef(strWidgetHandle.c_str()), doc.GetAllocator());
						jsParameters.AddMember(rapidjson::StringRef("WindowTop"), 0, doc.GetAllocator());
						jsParameters.AddMember(rapidjson::StringRef("WindowLeft"), 0, doc.GetAllocator());
						jsParameters.AddMember(rapidjson::StringRef("WindowWidth"), 1000, doc.GetAllocator());
						jsParameters.AddMember(rapidjson::StringRef("WindowHeight"), 900, doc.GetAllocator());

						jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
						jsActionContext.AddMember(rapidjson::StringRef("NotifyType"), rapidjson::StringRef("SetWidgetPreference"), doc.GetAllocator());
						jsActionContext.AddMember(rapidjson::StringRef("Notifier"), rapidjson::StringRef("00000000"), doc.GetAllocator());

						doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
						doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Notify"), doc.GetAllocator());
						doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

						rapidjson::StringBuffer strBuf;
						rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
						doc.Accept(writer);

						std::string data = strBuf.GetString();
						CCosmosApi::GetInstance()->Notify("SetWidgetPreference", data);
					}

					/*------------------------------*/
					/*       批量更新组件属性		*/
					/*     更新组件demo组件属性		*/
					/*------------------------------*/
					{
						std::this_thread::sleep_for(std::chrono::seconds(10));				//sleep几秒钟，看下移动效果
						rapidjson::Document doc;
						doc.SetObject();
						rapidjson::Value jsPreference(rapidjson::kObjectType), jsParameters(rapidjson::kArrayType), jsActionContext(rapidjson::kObjectType);
						rapidjson::Value jsValue(rapidjson::kObjectType);

						jsValue.AddMember(rapidjson::StringRef("WidgetHandle"), rapidjson::StringRef(strWidgetHandle.c_str()), doc.GetAllocator());
						jsValue.AddMember(rapidjson::StringRef("WindowTop"), 0, doc.GetAllocator());
						jsValue.AddMember(rapidjson::StringRef("WindowLeft"), 0, doc.GetAllocator());
						jsValue.AddMember(rapidjson::StringRef("WindowWidth"), 300, doc.GetAllocator());
						jsValue.AddMember(rapidjson::StringRef("WindowHeight"), 800, doc.GetAllocator());

						jsParameters.PushBack(jsValue, doc.GetAllocator());

						jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
						jsActionContext.AddMember(rapidjson::StringRef("NotifyType"), rapidjson::StringRef("SetRangeWidgetPreference"), doc.GetAllocator());
						jsActionContext.AddMember(rapidjson::StringRef("Notifier"), rapidjson::StringRef("00000000"), doc.GetAllocator());

						doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
						doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Notify"), doc.GetAllocator());
						doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

						rapidjson::StringBuffer strBuf;
						rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
						doc.Accept(writer);

						std::string data = strBuf.GetString();
						CCosmosApi::GetInstance()->Notify("SetRangeWidgetPreference", data);
					}

					/*------------------------------*/
					/*			 关闭组件	   		*/
					/*								*/
					/*------------------------------*/
					{
						std::this_thread::sleep_for(std::chrono::seconds(10));				//sleep几秒钟，看下效果
						rapidjson::Document doc;
						doc.SetObject();
						rapidjson::Value jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);

						jsParameters.AddMember(rapidjson::StringRef("WidgetHandle"), rapidjson::StringRef(strWidgetHandle.c_str()), doc.GetAllocator());
						jsParameters.AddMember(rapidjson::StringRef("WindowHandle"), rapidjson::StringRef(strWindowsHandle.c_str()), doc.GetAllocator());

						jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
						jsActionContext.AddMember(rapidjson::StringRef("Function"), rapidjson::StringRef("DestroyWidget"), doc.GetAllocator());
						jsActionContext.AddMember(rapidjson::StringRef("Invoker"), rapidjson::StringRef("00000000"), doc.GetAllocator());

						doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
						doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());
						doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

						rapidjson::StringBuffer strBuf;
						rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
						doc.Accept(writer);

						std::string data = strBuf.GetString();
						CCosmosApi::GetInstance()->Invoke("DestroyWidget", data);
					}

				}


                /*------------------------------*/
                /*			注册全局快捷键		*/
                /*								*/
                /*------------------------------*/
                {
					struct HotKey
					{
						uint32_t modifier;			// 修饰键
						uint32_t key;				// 虚拟键
					};

					struct ShortcutRequest
					{
						std::string ShortcutId;			// 快捷键唯一标识
						std::string Description;		// 快捷键描述
						HotKey shortcut;				// 键组合
					};

					std::vector<ShortcutRequest> requests;
                    ShortcutRequest r1 = { "1", "截图", {1, 65} }; // Alt + A
                    ShortcutRequest r2 = { "2", "锁定", {2, 76} }; // Ctrl + L
                    requests.push_back(r1);
                    requests.push_back(r2);

                    rapidjson::Document doc;
                    doc.SetObject();
                    rapidjson::Value jsParameters(rapidjson::kArrayType), jsActionContext(rapidjson::kObjectType);
					for (auto request : requests)
					{
                        rapidjson::Value jsShortcutRequest(rapidjson::kObjectType), jsHotKey(rapidjson::kObjectType);

						jsShortcutRequest.AddMember(rapidjson::StringRef("ShortcutId"), rapidjson::Value(request.ShortcutId.c_str(), doc.GetAllocator()), doc.GetAllocator());
						jsShortcutRequest.AddMember(rapidjson::StringRef("Description"), rapidjson::Value(request.Description.c_str(), doc.GetAllocator()), doc.GetAllocator());

						jsHotKey.AddMember(rapidjson::StringRef("Modifiers"), (int)request.shortcut.modifier, doc.GetAllocator());
						jsHotKey.AddMember(rapidjson::StringRef("Key"), (int)request.shortcut.key, doc.GetAllocator());
						jsShortcutRequest.AddMember(rapidjson::StringRef("ShortCut"), jsHotKey, doc.GetAllocator());
                        jsParameters.PushBack(jsShortcutRequest, doc.GetAllocator());
					}

                    jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
                    jsActionContext.AddMember(rapidjson::StringRef("Function"), rapidjson::StringRef("AddShortcut"), doc.GetAllocator());
                    jsActionContext.AddMember(rapidjson::StringRef("Invoker"), rapidjson::StringRef("00000000"), doc.GetAllocator());

                    doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
                    doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());
                    doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

                    rapidjson::StringBuffer strBuf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
                    doc.Accept(writer);

                    std::string data = strBuf.GetString();
                    CCosmosApi::GetInstance()->Invoke("AddShortcut", data);
                }

                /*------------------------------*/
                /*			获取全局快捷键		*/
                /*								*/
                /*------------------------------*/
                {
                    rapidjson::Document doc;
                    doc.SetObject();
                    rapidjson::Value jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);

                    jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
                    jsActionContext.AddMember(rapidjson::StringRef("Function"), rapidjson::StringRef("GetShortcut"), doc.GetAllocator());
                    jsActionContext.AddMember(rapidjson::StringRef("Invoker"), rapidjson::StringRef("00000000"), doc.GetAllocator());

                    doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
                    doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());
                    doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

                    rapidjson::StringBuffer strBuf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
                    doc.Accept(writer);

                    std::string data = strBuf.GetString();
                    std::string strResult = CCosmosApi::GetInstance()->Invoke("GetShortcut", data);

                }

                /*------------------------------*/
                /*			删除全局快捷键		*/
                /*								*/
                /*------------------------------*/
                {
                    struct HotKey
                    {
                        uint32_t modifier;			// 修饰键
                        uint32_t key;				// 虚拟键
                    };

                    struct ShortcutRequest
                    {
                        std::string ShortcutId;			// 快捷键唯一标识
                        std::string Description;		// 快捷键描述
                        HotKey shortcut;				// 键组合
                    };

                    std::vector<ShortcutRequest> requests;
                    ShortcutRequest r1 = { "1", "截图", {1, 65} }; // Alt + A
                    ShortcutRequest r2 = { "2", "锁定", {2, 76} }; // Ctrl + L
                    requests.push_back(r1);
                    requests.push_back(r2);

                    rapidjson::Document doc;
                    doc.SetObject();
                    rapidjson::Value jsParameters(rapidjson::kArrayType), jsActionContext(rapidjson::kObjectType);
                    for (auto request : requests)
                    {
                        rapidjson::Value jsShortcutRequest(rapidjson::kObjectType), jsHotKey(rapidjson::kObjectType);

                        jsShortcutRequest.AddMember(rapidjson::StringRef("ShortcutId"), rapidjson::Value(request.ShortcutId.c_str(), doc.GetAllocator()), doc.GetAllocator());
                        jsShortcutRequest.AddMember(rapidjson::StringRef("Description"), rapidjson::Value(request.Description.c_str(), doc.GetAllocator()), doc.GetAllocator());

                        jsHotKey.AddMember(rapidjson::StringRef("Modifiers"), (int)request.shortcut.modifier, doc.GetAllocator());
                        jsHotKey.AddMember(rapidjson::StringRef("Key"), (int)request.shortcut.key, doc.GetAllocator());
                        jsShortcutRequest.AddMember(rapidjson::StringRef("ShortCut"), jsHotKey, doc.GetAllocator());
                        jsParameters.PushBack(jsShortcutRequest, doc.GetAllocator());
                    }

                    jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
                    jsActionContext.AddMember(rapidjson::StringRef("Function"), rapidjson::StringRef("DelShortcut"), doc.GetAllocator());
                    jsActionContext.AddMember(rapidjson::StringRef("Invoker"), rapidjson::StringRef("00000000"), doc.GetAllocator());

                    doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
                    doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());
                    doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

                    rapidjson::StringBuffer strBuf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
                    doc.Accept(writer);

                    std::string data = strBuf.GetString();
                    CCosmosApi::GetInstance()->Invoke("DelShortcut", data);
                }

				/*------------------------------*/
				/*			更换主题颜色		*/
				/*								*/
				/*------------------------------*/
				{
					rapidjson::Document doc;
					doc.SetObject();
					rapidjson::Value jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);

					jsParameters.AddMember(rapidjson::StringRef("ColorScheme"), rapidjson::StringRef("Light"), doc.GetAllocator());

					jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
					jsActionContext.AddMember(rapidjson::StringRef("Function"), rapidjson::StringRef("SetUserExperience"), doc.GetAllocator());
					jsActionContext.AddMember(rapidjson::StringRef("Invoker"), rapidjson::StringRef("00000000"), doc.GetAllocator());

					doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
					doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());
					doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

					rapidjson::StringBuffer strBuf;
					rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
					doc.Accept(writer);

					std::string data = strBuf.GetString();
					CCosmosApi::GetInstance()->Invoke("SetUserExperience", data);
				}

				/*------------------------------*/
				/*			获取皮肤字典		*/
				/*								*/
				/*------------------------------*/
				{
					rapidjson::Document doc;
					doc.SetObject();
					rapidjson::Value jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);

					jsParameters.AddMember(rapidjson::StringRef("Color"), rapidjson::StringRef("Dark"), doc.GetAllocator());

					jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
					jsActionContext.AddMember(rapidjson::StringRef("Function"), rapidjson::StringRef("GetResouceDictionary"), doc.GetAllocator());
					jsActionContext.AddMember(rapidjson::StringRef("Invoker"), rapidjson::StringRef("00000000"), doc.GetAllocator());

					doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
					doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());
					doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

					rapidjson::StringBuffer strBuf;
					rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
					doc.Accept(writer);

					std::string data = strBuf.GetString();
					strResult = CCosmosApi::GetInstance()->Invoke("GetResouceDictionary", data);

					rapidjson::Document docResult;
					docResult.Parse(strResult.c_str());
					std::string strWidgetHandle;	//保存组件demo组件句柄
					std::string strWindowsHandle;	//保存组件demo窗口句柄,可以通过该句柄调用setparent方法内嵌到需要嵌入的窗口上
					if (!docResult.HasParseError())
					{
						auto nCode = docResult["Code"].GetInt64();
						//调用成功
						if (nCode == 200)
						{
							rapidjson::Document docTmp;
							docTmp.Parse(docResult["Data"].GetString());
							if (!docTmp.HasParseError())
							{
								rapidjson::StringBuffer strBufResult;
								rapidjson::Writer<rapidjson::StringBuffer> writerResult(strBufResult);
								docTmp["ActionContext"]["Return"].Accept(writerResult);
								std::string result = strBufResult.GetString();
								printf("GetResouceDictionary result:%s\n", result.c_str());
							}

						}
					}
				}

				/*------------------------------*/
				/*			 关闭组件引擎		*/
				/*								*/
				/*------------------------------*/
				{
					rapidjson::Document doc;
					doc.SetObject();
					rapidjson::Value jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);

					jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
					jsActionContext.AddMember(rapidjson::StringRef("Function"), rapidjson::StringRef("ShutdownCosmos"), doc.GetAllocator());
					jsActionContext.AddMember(rapidjson::StringRef("Invoker"), rapidjson::StringRef("00000000"), doc.GetAllocator());

					doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
					doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());
					doc.AddMember(rapidjson::StringRef("ActionInstance"), rapidjson::StringRef("HostDemo"), doc.GetAllocator());

					rapidjson::StringBuffer strBuf;
					rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
					doc.Accept(writer);

					std::string data = strBuf.GetString();
					//CCosmosApi::GetInstance()->Invoke("ShutdownCosmos", data);
				}
			}
		}
	});

	

	// 跨平台主循环：等待 Ctrl+C 退出
	while (!g_stop.load(std::memory_order_relaxed))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	/*------------------------------*/
	/*		释放宿主sdk内存对象		*/
	/*								*/
	/*------------------------------*/
	CCosmosApi::GetInstance()->Close();
    return 0;
}
