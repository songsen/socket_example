else if(strings.HasPrefix(msgIn.Content,"查询") == true){  
	sentence := strings.Replace(msgIn.Content,"查询","",1)  
	sentence = strings.TrimSpace(sentence)  
	msgback := utils.RobotApi(sentence + "的做法")  
} 

func CreateTransString(appid string,query string,salt string,key string,)string{  
	material := appid + query + salt + key  
	hasher := md5.New()  
	hasher.Write([]byte(material))  
	return hex.EncodeToString(hasher.Sum(nil))  
}  
	
func RobotApi(keymsg string)string{  
	url := "http://api.douqq.com/?key=PUVLKzdjeDduTWNHUFVXQUU3PWhRTytOekFrQUFBPT0&msg=" + keymsg  
	resp,err :=http.Get(url)  
	if err!=nil{  
		fmt.Print(err)  
	}  
	fetchrs,err :=ioutil.ReadAll(resp.Body)  
	defer resp.Body.Close()  
	return string(fetchrs)  
	
}