# ukagakaPlugin_mailBox
伺か/SSP用プラグインMailBox<br>
ゴーストからのメールが届くように(送れるようになる)プラグインです。<br>
目的のゴーストをたたせていないときでも届きます。<br>

このプラグインは、ゴーストがより人間らしいコミュニケーションをとれるようにするために開発されました。<br>
伺かアドベントカレンダー2023に制作されました。<br>
[【伺か】ゴーストがユーザにメールを送れるようになるプラグインを書いた。【SSP】](https://ambergonslibrary.com/?p=9188)<br>


## 一般ユーザ向け
プラグイン導入後、ゴーストの右クリック -> プラグイン -> MailBox を開くことによってメニューを開くことができます。<br>
以下項目<br>

- 未読メール<br>
- 既読メール<br>
- 個別メール<br>
    メニューを開いたゴーストの未読&既読メールを表示<br>

#### 自動機能
SSP起動後と約一時間おきに未通知メールの通知が入ります。<br>
通知をクリックすることで、未読ボックスを開くことができます。<br>


## ゴースト開発者向け
#### 機能一覧
このプラグインでは、下記の機能が提供されます。

- メールの送信
- メールの削除
- 送信済みのすべてのメールIDの取得
- 単一のメールの状態確認
- 複数のメールの状態確認
- プラグインの存在を通知する関数。


#### デバッグメニュー
- 未達のメールリストを閲覧する。


## 関数
#### 共通の知識
- すべてのメールはメールIDで管理されています。<br>
- メールIDは数字で管理します。<br>
    負数は使えません。<br>
- メールIDの自動管理機能が1.0.1で追加されました。<br>
    OnSendMail使用時、メールIDにautoもしくはAuto、AUTOのいずれかを指定することで、0から空いている数字を探して自動で割り当てます。<br>
- 同じメールIDでメールを送信すると既存のメールが上書きされます。<br>
- この際、上書きされたメールは未通知・未読状態になります。<br>
- 送信日が明日以降の場合は問題なく指定した日にユーザに届きます。<br>
- 同じメールIDでもゴーストメニューの名前が違う限り、区別して管理されます。<br>


#### メールの送信:OnSendMail
```
\![notifyplugin,MailBox,OnSendMail,メールのID,送信年,送信月,送信日,送信者名,メールタイトル,メール本文]
# v1.0.1 で追加 / Auto
\![notifyplugin,MailBox,OnSendMail,Auto,送信年,送信月,送信日,送信者名,メールタイトル,メール本文]
```

- notifypluginを使用して送信します。
- メールは到着する日を指定して送信します。
- 今日や過去の日付に送るとその日からメールを確認できます。
- ,,の間にスペースを開けずに入力してください。
- 時刻の指定は予定しておりません。

日付の指定は下記のSAORIを使うと快適になるでしょう。
- [GitHub - ambergon/ukagakaSaori_CalcCalendarClang](https://github.com/ambergon/ukagakaSaori_CalcCalendarClang)
- [GitHub - ambergon/ukagakaSaori_DiffCalendarClang](https://github.com/ambergon/ukagakaSaori_DiffCalendarClang)

```
//さとり例文
：送信\![notifyplugin,MailBox,OnSendMail,0,（現在年）,（現在月）,（現在日）,送信者,タイトル,本文]
//yaya例文
"送信\![notifyplugin,MailBox,OnSendMail,0,%(year),%(month),%(day),送信者,タイトル,本文]"
```
メール内容にサクラスクリプトのリンクなどを組み込むことは可能ですが、メールの表示は現在表示中のゴーストに使用されるため、あまり使用する機会はないでしょう。<br>
サクラスクリプト側の仕様で一部の記号などが使えなかったりするので、手元で試してから使用してください。<br>
下記の例だと`「]」`を`「\]」`にして送信しています。<br>
```
//yaya
"\![raiseplugin,MailBox,OnSendMail,0,%(year),%(month),%(day),送信者,タイトル,無事に届いていますか?\n\_a[OnXX\]LINK\_a]"
```
写真付きのメールを送る。<br>
ghost/masterフォルダにred.pngをおいている場合、下記のようにすると`\_b`を活用して送信できます。<br>
```
＄PATH	（replace,（pwd）,\,/）
：画像送信\![notifyplugin,MailBox,OnSendMail,0,（現在年）,（現在月）,（現在日）,送信者,画像送信テスト,\_b[（PATH）red.png\,inline\,opaque\]]
```


#### 上書きしないメールの送信:OnSendMailNotUpdate
```
\![notifyplugin,MailBox,OnSendMailNotUpdate,メールのID,送信年,送信月,送信日,送信者名,メールタイトル,メール本文]
```
引数はOnSendMailと同じ。<br>
ただし、メールが存在しないときのみ送信される。<br>


#### 届いていないときのみメールの送信:OnSendMailNotArrive
```
\![notifyplugin,MailBox,OnSendMailNotArrive,メールのID,送信年,送信月,送信日,送信者名,メールタイトル,メール本文]
```
引数はOnSendMailと同じ。<br>
ただし、メールが存在しない時とまだ届いていないときのみ送信される。<br>
届いていない場合は内容と到着タイミングが更新される。<br>


#### メールの削除:OnDeleteMail
```
\![notifyplugin,MailBox,OnDeleteMail,メールID]
```
notifypluginを使用して削除します。<br>
主な用途は、一週間以上起動していなかった場合にメールを送信したい時、<br>
それ以前に起動された場合に削除するなどでしょう。<br>


#### 未達ならばメール削除:OnDeleteMailNotArrive
```
\![notifyplugin,MailBox,OnDeleteMailNotArrive,メールID]
```
- notifypluginを使用して実行。
- まだ届いていない場合のみ指定したメールを削除する。
- OnMailsStatus を経由せずに処理できる利点がある。


#### ゴーストが送信済みのすべてのメールIDの取得:OnGetAllMailID
raisepluginを使用して呼び出す。

呼び出し
```
\![raiseplugin,MailBox,OnGetAllMailID]
```
受け取り

- 返り値はID:ID:ID...で取得。
- [:]は半角文字です。
```
＃さとり
＊OnAllMailID 
：すべての使用済みID = （R0） 

//yaya
OnAllMailID {
    reference[0]
}
```


#### メールの状態を確認:OnStatusMail
raisepluginを使用して呼び出す。<br>
指定したメールIDのステータスを確認することができます。<br>
これを実行すると`OnMailStatus`関数がゴーストに呼ばれます。<br>

呼び出し。
```
\![raiseplugin,MailBox,OnStatusMail,メールID]
```
受け取り

- Reference0 : 確認したメールID
- Reference1 : そのメールのステータス
    - 0 : メールが存在しない
    - 1 : メールがまだ届いていない
    - 2 : 届いているが未読
    - 3 : 届いていて  既読
```
＃さとり
＊OnMailStatus
：MailID   =（R0）
MailStatus =（R1）

//yaya
OnMailStatus {
    _text = "mailID : " + reference[0] + "\n" + "status : " + reference[1]
    _text
}
```


#### メールの状態を確認EX:OnStatusMailEX
OnStatusMailと同等だが、任意の引数を5つ横流しすることができる。<br>
使用するとOnMailStatusEXが呼ばれる。<br>
横流しはOnBoot系の関数と同じ配置にするために以下のようになった。<br>
ただし、OnGhostChangedなどのR1(切り替え時のスクリプト)などを直接渡すと、`\e`が入っていたりでバグの素だったりする。適時置換して使うこと。<br>

呼び出し
```
\![raiseplugin,MailBox,OnStatusMailEX, 横流し0, 横流し1, 横流し2, 横流し3, 横流し4, メールID , 横流し5]
```
受け取り

- Reference 0: 横流し0 
- Reference 1: 横流し1 
- Reference 2: 横流し2 
- Reference 3: 横流し3 
- Reference 4: 横流し4 
- Reference 5: メールID 
- Reference 6: メールステータス
- Reference 7: 横流し5 
```
＃さとり
＊OnMailStatusEX
： R0 = （R0）
R1 = （R1）
R2 = （R2）
R3 = （R3）
R4 = （R4）
R5 = （R5）

//yaya
OnMailStatusEX {
    _text = "mailID : " + reference[0] + "\n" + "status : " + reference[1] + "\nR2 : " + reference[2] + "\nR3 : " + reference[3] + "\nR4 : " + reference[4] + "\nR5 : " + reference[5] + "\nR6 : " + reference[6]
    _text
}
```


#### 複数のメールの状態確認:OnStatusMails
呼び出し

メールIDを半角[:]区切りでメールステータスを複数要求できます。
```
//OnStatusMail[s]
\![raiseplugin,MailBox,OnStatusMails,0:1:2:3:4:5]
```
受け取り
同じく半角[:]区切りでステータスが返ってきます。

- Reference 0: 問い合わせたID詰め合わせ
- Reference 1: 問い合わせたステータス詰め合わせ
```
＃OnMail[s]Status
＃さとり
＊OnMailsStatus
：要求したIDs = （R0） 
：要求したステータスs = （R1） 

//yaya
//OnMail[s]Status
OnMailsStatus {
    _text = "mailIDs : " + reference[0] + "\n" + "status : " + reference[1]
    _text
}
```


#### プラグインの存在を通知する関数。:OnExistPluginMailBox
reference:無し<br>
OnBoot直後にゴーストにOnExistPluginMailBoxイベントを送ります。<br>
フラグ管理をすることで、MailBoxがインストール済みかどうかの処理ができます。<br>
(OnBootには間に合わない)<br>
<br>
里々だとこのようにすれば、検知できるでしょう。<br>
```
＃＃ プラグインの存在検知 = ExistPluginMailBox
＃＃ ExistPluginMailBox = 1 ： 有効
＃＃ ExistPluginMailBox = 0 ： 無効

＃ 各起動関数に追記
＊OnBoot
＄ExistPluginMailBox=0

＃ 起動後にチェック関数が飛ぶ。
＊OnExistPluginMailBox
＄ExistPluginMailBox=1
```
もしくはこのようにする。<br>
起動時にOnStatusMailEX関数をプラグインの有無を確認しつつ使用する場合、<br>
OnExistPluginMailBox関数が間に合わないためこのように実装するなどした。<br>
(そもそもその場合はプラグイン必須になってしまうだろうから有無の確認はいらないかもしれない。)<br>
```
＃初期化処理
＊OnSatoriBoot
＄ExistPluginMailBoxFlag=0
＄ExistPluginMailBoxFlagOnce=0

＃ 現在バグで二回呼ばれることがある為このようになった。
＊OnSatoriClose
（when,（ExistPluginMailBoxFlagOnce）==0&&（ExistPluginMailBoxFlag）==1,（set,ExistPluginMailBox,1）,）
（when,（ExistPluginMailBoxFlagOnce）==0&&（ExistPluginMailBoxFlag）==0,（set,ExistPluginMailBox,0）,）
＄ExistPluginMailBoxFlagOnce=1

＃実行タイミングはBoot後少し後。
＊OnExistPluginMailBox
＄ExistPluginMailBox=1
＄ExistPluginMailBoxFlag=1
```


## 未達のメールリストを閲覧する。
この機能は通常呼び出すことができません。<br>
任意のゴーストから、下記を実行してください。<br>
```
\![raiseplugin,MailBox,OnDevList,0]
```


## Other
このプラグインは無保証で配布されます。<br>
リンク、同梱などは問題ありません。<br>
好きに使ってください。<br>
ポケベル・携帯・スマホ・手紙、などゴーストの時代背景にあったツールとして使ってください。<br>


## Author
ambergon



