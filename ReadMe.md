# ukagakaPlugin_mailBox
このプラグインは、ゴーストがより人間らしいコミュニケーションをとれるようにするために開発されました。


## 一般ユーザの使用
#### 起動時のメール確認
起動時からN秒後に未読メールのチェックを行います。
descript.txtにある、secondchangeintervalの数字を任意の秒数に変更してください。
デフォルトでは3分後です。
```
secondchangeinterval,180
```

#### メールを確認する。
プラグインメニューからMailBoxを呼ぶと未読メール・既読メールの確認ができます。


## ゴースト作者の使用
このプラグインでは、下記の機能が提供されます。

- メールの送信
- メールの削除
- メールの状態確認


#### 注意事項
各ゴーストのメールはゴーストのメニュー名とメールIDを使用して管理しています。
後述しますが、同じメールIDを用いることで、そのメールの状態・削除などを管理するので、送信の際は重複に気を付けてください。
同じメールIDで送信された場合、過去のものを削除したうえで、新しいメールの処理を行います。

また、日付やメールIDは半角の数字で入力する必要があります。


#### メールの送信
メールは到着する日を指定して送信します。
今日や過去の日付に送るとその日からメールを確認できます。
時刻の指定は予定しておりません。

日付の指定は下記のSAORIを使うと快適になるでしょう。

- [GitHub - ambergon/ukagakaSaori_CalcCalendar](https://github.com/ambergon/ukagakaSaori_CalcCalendar)
- [GitHub - ambergon/ukagakaSaori_DiffCalendar](https://github.com/ambergon/ukagakaSaori_DiffCalendar)

```
//サクラスクリプト
"\![raiseplugin,MailBox,OnSendMail,メールのID,送信年,送信月,送信日,送信者名,メールタイトル,メール本文]"
//例文
"\![raiseplugin,MailBox,OnSendMail,0,%(year),%(month),%(day),琥珀,初めまして,無事に届いていますか?]"
```

メール内容にサクラスクリプトのリンクなどを組み込むことは可能ですが、メールの表示は現在表示中のゴーストに使用されるため、あまり使用する機会はないでしょう。
サクラスクリプト側の仕様で一部の記号などが使えなかったりするので、手元で試してから使用してください。
下記の例だと`「]」`を`「\]」`にして送信しています。
```
"\![raiseplugin,MailBox,OnSendMail,0,%(year),%(month),%(day),琥珀,初めまして。,無事に届いていますか?\n\_a[OnXX\]LINK\_a]"
```


#### メールの削除
例えば、最後の起動からN日後に送信するようにしたとします。
それより早く起動された場合などに使用することを想定しています。
```
"\![raiseplugin,MailBox,OnDeleteMail,メールID]"
```


#### メールの状態チェック
指定したメールIDのステータスを確認することができます。
これを実行すると`OnMailStatus`関数がゴーストに呼ばれます。

```
//確認用関数
"\![raiseplugin,MailBox,OnStatusMail,メールID]"


////reference[0]
//メールID
////reference[1]
//0 : メールが存在しない
//1 : メールがまだ届いていない
//2 : 届いているが未読
//3 : 届いていて  既読
OnMailStatus {
    reference[0]
    reference[1]
}
```


## Other
このプラグインは無保証で配布されます。
リンク、同梱などは問題ありません。
好きに使ってください。
12/25日に正式公開予定です。


## Author
ambergon





