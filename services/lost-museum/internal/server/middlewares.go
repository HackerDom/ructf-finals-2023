package server

import (
	"errors"
	"github.com/gofiber/fiber/v2"
	"net/http"
	"ructf-2023-finals/lost-museum/internal/infra"
	"strings"
)

func AuthMiddleware(secret string) fiber.Handler {
	return func(c *fiber.Ctx) error {
		token, ok := c.GetReqHeaders()["Authorization"]
		if !ok {
			c.Status(http.StatusUnauthorized)
			return c.JSON(errorResponse(errors.New("missing authorization header")))
		}

		token = strings.Replace(token, "Bearer ", "", 1)
		claims, err := infra.DecodeToken(token, secret)
		if err != nil {
			c.Status(http.StatusForbidden)
			return c.JSON(errorResponse(errors.New("forbidden")))
		}

		c.Locals("username", claims["username"])

		return c.Next()
	}
}
