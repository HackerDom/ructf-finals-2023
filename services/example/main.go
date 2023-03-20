package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
)

func main() {
	usernameToFlags := make(map[string]string)
	usernameToTokens := make(map[string]string)

	http.HandleFunc("/check", func(w http.ResponseWriter, r *http.Request) {})

	http.HandleFunc("/put", func(w http.ResponseWriter, r *http.Request) {
		data, err := ioutil.ReadAll(r.Body)
		if err != nil {
			w.WriteHeader(500)
			_, _ = fmt.Fprintf(w, "reading request body error: "+err.Error())
			return
		}

		var putRequest struct {
			Username string `json:"username"`
			Token    string `json:"Token"`
			Flag     string `json:"flag"`
		}

		if err := json.Unmarshal(data, &putRequest); err != nil {
			w.WriteHeader(400)
			_, _ = fmt.Fprintf(w, "unmarshal error: "+err.Error())
			return
		}

		if putRequest.Flag == "" || putRequest.Username == "" || putRequest.Token == "" {
			w.WriteHeader(400)
			_, _ = fmt.Fprintf(w, "invalid put json")
			return
		}

		usernameToFlags[putRequest.Username] = putRequest.Flag
		usernameToTokens[putRequest.Username] = putRequest.Token
	})

	http.HandleFunc("/get", func(w http.ResponseWriter, r *http.Request) {
		checkParam := func(name string) (string, error) {
			v, ok := r.URL.Query()[name]
			if !ok || len(v) == 0 {
				return "", errors.New(fmt.Sprintf("invalid query params: %v is required", name))
			}
			return v[0], nil
		}
		username, err := checkParam("username")
		if err != nil {
			w.WriteHeader(400)
			_, _ = w.Write([]byte(err.Error()))
			return
		}

		token, err := checkParam("token")
		if err != nil {
			w.WriteHeader(400)
			_, _ = w.Write([]byte(err.Error()))
			return
		}

		actualToken, ok1 := usernameToTokens[username]
		flag, ok2 := usernameToFlags[username]

		if !ok1 || !ok2 {
			w.WriteHeader(400)
			_, _ = w.Write([]byte("user does not exist"))
			return
		}

		if actualToken != token {
			w.WriteHeader(401)
			_, _ = w.Write([]byte("invalid token"))
			return
		}

		_, _ = w.Write([]byte(flag))
	})


  http.HandleFunc("/hack", func(w http.ResponseWriter, r *http.Request) {
    checkParam := func(name string) (string, error) {
      v, ok := r.URL.Query()[name]
      if !ok || len(v) == 0 {
        return "", errors.New(fmt.Sprintf("invalid query params: %v is required", name))
      }
      return v[0], nil
    }
    username, err := checkParam("username")
    if err != nil {
      w.WriteHeader(400)
      _, _ = w.Write([]byte(err.Error()))
      return
    }

    flag, ok := usernameToFlags[username]

    if !ok {
      w.WriteHeader(400)
      _, _ = w.Write([]byte("user does not exist"))
      return
    }

    _, _ = w.Write([]byte(flag))
  })

	log.Fatal(http.ListenAndServe(":2222", nil))
}
