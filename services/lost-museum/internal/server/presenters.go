package server

import (
	"github.com/gofiber/fiber/v2"
)

func emptySuccessResponse() *fiber.Map {
	return &fiber.Map{"success": true}
}

func errorResponse(err error) *fiber.Map {
	return &fiber.Map{"error": err.Error()}
}

type authRequest struct {
	Username string `form:"username"`
	Password string `form:"password"`
}

func authSuccessResponse(token string) *fiber.Map {
	return &fiber.Map{
		"token": token,
	}
}

type getJokeRequest struct {
	Theme string `form:"theme"`
}

type createJokeRequest struct {
	Theme string `form:"theme"`
	Text  string `form:"text"`
}

func jokeSuccessResponse(v map[string]string) *fiber.Map {
	return &fiber.Map{
		"theme":  v["theme"],
		"text":   v["text"],
		"status": v["status"],
	}
}

func jokesListSuccessResponse(v []map[string]string) *fiber.Map {
	data := make([]map[string]string, 0, len(v))

	for _, j := range v {
		data = append(data, map[string]string{
			"theme":  j["theme"],
			"text":   j["text"],
			"status": j["status"],
		})
	}

	return &fiber.Map{"data": data}
}
