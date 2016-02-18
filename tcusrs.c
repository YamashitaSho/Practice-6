#include <stdio.h>
#include <string.h>

/*
C言語学習用課題
ファイルから文字列を取得し、構造体に格納する。

ファイル名	user.txt

ファイル内容の並びは「氏名」「かな」「電話番号」で構成。ファイルフォーマットは CSV.
1ファイル最大100名まで格納されている。

内容例
木村拓哉,きむらたくや,075-111-2221
稲垣五郎,いながきごろう,075-111-2222
香取慎吾,かとりしんご,075-111-2223

エラーチェック
ファイルが見つからない
ファイルフォーマットエラー
その他のエラー

コマンドライン
tcusers

*/
#define DATA_NUM 100
#define FILENAME "user.txt"
#define DEFAULT_WORD "-"

#define NO_ERROR 0
#define FILE_ERROR 1
#define FILETYPE_ERROR 2
#define NO_WORD_ERROR 3
#define UNKNOWN_ERROR -1

#define MODE_MENU '0'
#define MODE_LOAD '1' 
#define MODE_SEARCH '2'
#define MODE_EXIT '3'

#define MATCHING_FORMAT '0'
#define MATCHING_FOUND '1'

typedef struct _user user_t;
struct _user{
	char name[50];
	char ruby[50];
	char phone[20];
};

void struct_init(user_t *p);				//構造体配列の初期化(先頭要素のポインタを受け取る)
int main_loop(user_t *p ,char *file);		//メニューを入力させて各処理に移行する
char get_menu();							//メニューを入力させる

int struct_load(user_t *p ,char *file);		//構造体にファイルをロードする
void input_a_line(char *file, char mode);	//ファイル名を入力させる

void struct_print(user_t *p);				//構造体の内容を表示する
int struct_search(user_t *p);				//検索
int check(char *data, char *keyword);		//検索本体
void search_result(user_t *p, char *matching, int matchnumber);
											//検索結果を表示する

int error_msg(int err);						//エラーがある場合はエラーメッセージを出し、致命的なエラーは返り値にNO_ERROR以外を返す

//main関数-----------------------------------------------------------------------
int main(){
	char file[256] = FILENAME;

	user_t data[DATA_NUM];
	user_t *data_ptr;
	data_ptr = data;
	struct_init(data_ptr);
//	printf("12000になればOK:%lu\n",sizeof(*data_ptr));
//	memset(data_ptr , '\0', sizeof(*data_ptr));
	
	main_loop(data_ptr , file);
	
	return 0 ;
}
//mainここまで--------------------------------------------------------------------

//構造体の配列を空にする(ポインタから参照したmemsetでは配列番号[0]しか空にならなかった)-----
//[0]を空にして残りに[0]を代入する---------------------------------------------------
void struct_init(user_t *data){
	int i = 0;
	memset(data->name,'\0',50);
	memset(data->ruby,'\0',50);
	memset(data->phone,'\0',20);
	for( i = 1; i < DATA_NUM ; i++){
		data[i] = data[0];
	}
}

//メインループ--------------------------------------------------------------------
int main_loop(user_t *data_ptr ,char *file){
	int err = NO_ERROR;
	char mode = MODE_MENU;
	while( !0 ){
		mode = get_menu();
		
		if ( mode == MODE_LOAD ){				//読み込みモード
			struct_init(data_ptr);				//構造体の再初期化
			err = struct_load(data_ptr , file);
//			struct_print(data_ptr);				//構造体の内容を表示する
		}
		
		if ( mode == MODE_SEARCH ){				//検索モード
			err = struct_search(data_ptr);			//検索
		}
		if ( mode == MODE_EXIT ){				//終了
			break;
		}
		if (error_msg(err) != NO_ERROR) {		//エラー終了
			break;
		}
	}
	return 0;
}

//メニュー表示・入力---------------------------------------------------------------
	
char get_menu(){
	char input[50];
	char mode;
	printf("1.ファイル読み込み\n2.検索\n3.終了\nメニュー番号を入力してください:");
	fflush(stdin);
	fgets(input,50,stdin);
	fflush(stdin);
	mode = input[0];
	
	return mode;
}

//モードごとにメッセージを出しつつ1行入力させる-----------------------------------------
void input_a_line(char *inputline, char mode){
	char input[256];
	char *newline;									//改行検出用
	
	if (mode == MODE_LOAD){
		printf("\n読み込むファイル名を入力してください。\n何も入力せずにEnterを押すと%sが読み込まれます。\n>>", FILENAME);
	} else if (mode == MODE_SEARCH) {
		printf("検索するワードを入力してください。\n>>");
	}
	fflush(stdin);
	fgets(input , 255 , stdin);
	
	if (input[0] != '\n') {
		newline = memchr(input , '\n', 255);		//fileの終端にある改行コードを検出する
		*newline = '\0';							//'\0'に置き換える
		strncpy(inputline , input , 255);
	}
}

//FILENAMEのファイルを構造体pに読み込む----------------------------------------------
int struct_load(user_t *p ,char *file){
	int i = 0;
	char buf[120];
	char *buf_p;
	FILE *fp;
	
	memset( buf , '\0' , 120 );					//buf[119]まで'\0'埋め
	
	input_a_line(file , MODE_LOAD);				//ファイル名をユーザーに要求する
	
	fp = fopen(file,"r");
	if (fp == NULL){
		printf("%s:ファイルが開けませんでした。\n", file);
		return FILE_ERROR;
	}
	
	while(fgets(buf , 119 , fp) != NULL){		//NULLまで1行ごとに読み込む(最大119バイト→120バイト目は必ず'\0'になっている→strtokでバッファオーバーフローする心配はない)
		
		if ((memchr(buf, '\n', 119) == 0) && (memchr(buf, '\0', 119) == 0)){
			return FILETYPE_ERROR;				//bufの120バイト目までに改行コードまたはNULLがない場合エラー
		}
		if (buf[0] == '\0'){
			break;
		}

		buf_p = strtok( buf , "," );			//もし","がいなくても119バイトで止まる
		if (buf_p != 0) {						//","が見つかっていた場合
			strncpy( (p+i)->name , buf_p ,49);	//buf_pが示す値を49バイトまでコピー(50バイトまで初期化してある)
		} else {								//見つからないならファイル形式エラー
			return FILETYPE_ERROR;
		}

		buf_p = strtok( NULL , "," );
		if (buf_p != 0) {
			strncpy( (p+i)->ruby , buf_p ,49);
		} else {
			return FILETYPE_ERROR;
		}

		buf_p = strtok( NULL , "\n" );			//構造体の3つ目の区切り文字は改行コード
		if (buf_p != 0){
			strncpy( (p+i)->phone , buf_p ,19);
		} else {
			return FILETYPE_ERROR;
		}
		i++;
		if ( i >= DATA_NUM ){
			break;								//DATA＿NUM個受け取ったら終わり
		}
		memset( buf , '\0' , 120 );				//一周ごとに再度'\0'で埋める
	}
	
	fclose(fp);
	printf("ファイルを読み込みました:%s\n\n", file);
	return NO_ERROR;
}

//構造体の内容を表示する------------------------------------------------------------
void struct_print(user_t *p){
	int i = 0;
	printf("\n構造体の内容を表示します。\n");
	while(strcmp((p+i)->name,"\0") != 0){
		printf("氏名:%s\nかな:%s\nTEL :%s\n",(p+i)->name,(p+i)->ruby,(p+i)->phone);
		i++;
		if (i > DATA_NUM){
			break;
		}
	}
}

//構造体の内容から検索を行う---------------------------------------------------------
int struct_search(user_t *p){
	char keyword[256] = DEFAULT_WORD;
	char matching[DATA_NUM+1];						//発見データ
	int i = 0;										//構造体配列番号と共に動くループ変数
	int matchnumber = 0;							//発見回数
	
	memset(matching,MATCHING_FORMAT,DATA_NUM+1);
	memset(keyword,'\0',256);
	
	input_a_line(keyword , MODE_SEARCH);			//ワードをもらってくる
	if (strlen(keyword) == 0){
		return NO_WORD_ERROR;
	}
	while(strcmp((p+i)->name,"\0") != 0){
		
		matching[i] = check((p+i)->name, keyword);
		
		if (matching[i] == MATCHING_FORMAT){
			matching[i] = check((p+i)->ruby, keyword);
		}
		
		if (matching[i] == MATCHING_FORMAT){
			matching[i] = check((p+i)->phone, keyword);
		}
		if (matching[i] == MATCHING_FOUND){
			matchnumber ++;
		}
		
		i++;
		if (i > DATA_NUM-1){						// DATA_NUM-1 番目までしか存在しない
			break;
		}
	}
	
//	printf("search:%s\n", keyword);										//検索ワード
//	for (int m=0;m<DATA_NUM;m++){printf("matching:%c\n",matching[m]);}	//マッチングデータの表示(長い)
	search_result( p , matching , matchnumber);
	return 0;
}
//検索本体-----------------------------------------------------------------------
//data内にkeywordがあればMATCHING＿FOUNDを、なければMATCHING_FORMATを返す
int check(char *data, char *keyword){
	int datalength = strlen(data);
	int wordlength = strlen(keyword);
	int i ;					//dataに対するkeyword先頭文字のインデックス
	int j ;					//keyword内のインデックス
	int searchmax = datalength - wordlength + 1;
	
	if (searchmax >= 1){							//検索回数が0以下でない
		for(i = 0; i < searchmax ; ) {				//ハズレまたは発見が確定した時にj++
			if (data[i] == keyword[0]) {			//1文字目が一致
				for(j = 0; j < wordlength ; j++){
					if (data[i+j] != keyword[j]) {
						i++;						//2文字目以降でハズレ
						break;
					}
					if ((data[i+j] == keyword[j]) & (j == wordlength - 1)) {
						return MATCHING_FOUND;		//発見
					}
				}
			} else {								//1文字目もハズレ
				i++;
			}
		}
	}
	return MATCHING_FORMAT;
}

//検索結果の表示-------------------------------------------------------------------
//構造体の配列のうち、一致するものがあった要素を表示する
void search_result(user_t *p, char *matching, int matchnumber){
	int i = 0;
	if (matchnumber > 0){
		printf("\nファイルの検索結果を表示します。\n\n");
		while(strcmp((p+i)->name,"\0") != 0){
			if (matching[i] != MATCHING_FORMAT){
				printf("氏名:%s\nかな:%s\nTEL :%s\n",(p+i)->name,(p+i)->ruby,(p+i)->phone);
			}
			i++;
			if (i > DATA_NUM){
				break;
			}
		}
		printf("\n");
	} else {
		printf("\nワードは見つかりませんでした。\n\n");
		return;
	}
}


//エラーメッセージ-----------------------------------------------------------------
int error_msg(int err){
	switch (err){
	  case NO_ERROR:
		break;
		
	  case FILE_ERROR:
		printf("ファイル読み込みエラーです。\nEnterを押すとメインメニューに戻ります。\n");
		getc(stdin);
		fflush(stdin);
		break;
		
	  case FILETYPE_ERROR:
		printf("ファイル形式エラーです。\nEnterを押すとメインメニューに戻ります。\n");
		getc(stdin);
		fflush(stdin);
		break;
	  case NO_WORD_ERROR:
		printf("検索ワードを入力してください。\nEnterを押すとメインメニューに戻ります。\n");
		getc(stdin);
		fflush(stdin);
		break;
		
	  default:
		printf("不明なエラーが発生しました。\n");
		return UNKNOWN_ERROR;
	}
	return NO_ERROR;
}