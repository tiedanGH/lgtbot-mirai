// 注意: 本项目的所有源文件都必须是 UTF-8 编码

// 这是一个“反撤回”机器人
// 在群里回复 “/anti-recall enabled.” 或者 “撤回没用” 之后
// 如果有人在群里撤回，那么机器人会把撤回的内容再发出来

#include <iostream>
#include <map>
#include <mirai.h>
#include "myheader.h"
#include <gflags/gflags.h>
#include "lgtbot/BotCore/bot_core.h"
#include "lgtbot/Utility/msg_sender.h"

DEFINE_string(ip, "127.0.0.1", "The IP address");
DEFINE_int32(port, 8080, "The port");
DEFINE_string(auth, "", "The AuthKey for mirai-api-http");
DEFINE_int32(thread, 4, "The number of threads");
DEFINE_uint64(qq, 0, "Bot's QQ ID");

static Cyan::MiraiBot* g_bot_p = nullptr;

class MyMsgSender : public MsgSender
{
 public:
  MyMsgSender(const Target target, const uint64_t id) : target_(target), id_(id) {}
  virtual ~MyMsgSender() override
  {
    if (target_ == TO_USER)
    {
      g_bot_p->SendMessage(Cyan::QQ_t(id_), msg_);
    }
    else if (target_ == TO_GROUP)
    {
      g_bot_p->SendMessage(Cyan::GID_t(id_), msg_);
    }
  }
  virtual void SendString(const char* const str, const size_t len) override { msg_.Plain(std::string_view(str, len)); }
  virtual void SendAt(const uint64_t uid) override
  {
    if (target_ == TO_USER)
    {
      msg_.Plain(std::to_string(uid));
    }
    else if (target_ == TO_GROUP)
    {
      msg_.At(Cyan::QQ_t(uid));
    }
  }

 private:
  const Target target_;
  const UserID id_;
  Cyan::MessageChain msg_;
};

MsgSender* new_msg_sender(const Target target, const uint64_t id) { return new MyMsgSender(target, id); }
void delete_msg_sender(MsgSender* msg_sender) { delete msg_sender; }

int main(int argc, char** argv)
{
#if defined(WIN32) || defined(_WIN32)
	// 切换代码页，让 CMD 可以显示 UTF-8 字符
	system("chcp 65001");
#endif
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  BOT_API::Init(FLAGS_qq, new_msg_sender, delete_msg_sender, argc, argv);
  std::cout << "[QQ] " << FLAGS_qq << std::endl;
  std::cout << "[Address] " << FLAGS_ip << ":" << FLAGS_port << std::endl;
  Cyan::MiraiBot bot(FLAGS_ip, FLAGS_port, FLAGS_thread);
  g_bot_p = &bot;
	while (true)
	{
		try
		{
			bot.Auth(FLAGS_auth, Cyan::QQ_t(FLAGS_qq));
			break;
		}
		catch (const std::exception& ex)
		{
      std::cout << ex.what() << std::endl;
		}
    Cyan::MiraiBot::SleepSeconds(1);
	}
  std::cout << "Bot Working..." << std::endl;

  bot.On<Cyan::GroupMessage>([](Cyan::GroupMessage m)
    {
      try
      {
        if (m.AtMe()) { BOT_API::HandlePublicRequest(m.Sender.QQ, m.Sender.Group.GID, m.MessageChain.GetPlainText().c_str()); }
      }
			catch (const std::exception& ex) { std::cout << ex.what() << std::endl; }
    });

  bot.On<Cyan::FriendMessage>([](Cyan::FriendMessage m)
    {
      try
      {
        BOT_API::HandlePrivateRequest(m.Sender.QQ, m.MessageChain.GetPlainText().c_str());
      }
			catch (const std::exception& ex) { std::cout << ex.what() << std::endl; }
    });

  bot.On<Cyan::TempMessage>([](Cyan::TempMessage m)
    {
      try
      {
        BOT_API::HandlePrivateRequest(m.Sender.QQ, m.MessageChain.GetPlainText().c_str());
      }
			catch (const std::exception& ex) { std::cout << ex.what() << std::endl; }
    });

	bot.EventLoop([](const char* err_msg)
		{
      std::cout << "[bot错误] " << err_msg << std::endl;
		});

	return 0;
}
