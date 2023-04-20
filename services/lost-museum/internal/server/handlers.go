package server

import (
	"github.com/gofiber/fiber/v2"
	"net/http"
	"ructf-2023-finals/lost-museum/internal/app"
	"ructf-2023-finals/lost-museum/internal/common"
)

func Register(u app.AuthUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		var body authRequest

		if err := c.BodyParser(&body); err != nil {
			c.Status(http.StatusBadRequest)
			return c.JSON(errorResponse(err))
		}

		token, err := u.Register(c.Context(), body.Username, body.Password)
		if err != nil {
			switch err {
			case common.ErrAlreadyExists:
				c.Status(http.StatusBadRequest)
			default:
				c.Status(http.StatusInternalServerError)
			}
			return c.JSON(errorResponse(err))
		}

		return c.JSON(authSuccessResponse(token))
	}
}

func Login(u app.AuthUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		var body authRequest

		if err := c.BodyParser(&body); err != nil {
			c.Status(http.StatusBadRequest)
			return c.JSON(errorResponse(err))
		}

		token, err := u.Login(c.Context(), body.Username, body.Password)
		if err != nil {
			switch err {
			case common.ErrNotExists:
				c.Status(http.StatusBadRequest)
			case common.ErrInvalidPassword:
				c.Status(http.StatusForbidden)
			default:
				c.Status(http.StatusInternalServerError)
			}
			return c.JSON(errorResponse(err))
		}

		return c.JSON(authSuccessResponse(token))
	}
}

func GetJoke(u app.JokesUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		var body getJokeRequest

		if err := c.BodyParser(&body); err != nil {
			c.Status(http.StatusBadRequest)
			return c.JSON(errorResponse(err))
		}

		joke, err := u.GetJoke(c.Context(), c.Locals("username").(string), body.Theme)
		if err != nil {
			switch err {
			case common.ErrNotExists:
				c.Status(http.StatusNotFound)
			default:
				c.Status(http.StatusInternalServerError)
			}
			return c.JSON(errorResponse(err))
		}

		return c.JSON(jokeSuccessResponse(joke))
	}
}

func CreateJoke(u app.JokesUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		var body createJokeRequest

		if err := c.BodyParser(&body); err != nil {
			c.Status(http.StatusBadRequest)
			return c.JSON(errorResponse(err))
		}

		err := u.CreateJoke(c.Context(), c.Locals("username").(string), body.Theme, body.Text)
		if err != nil {
			switch err {
			default:
				c.Status(http.StatusInternalServerError)
			}
			return c.JSON(errorResponse(err))
		}

		c.Status(http.StatusCreated)
		return c.JSON(emptySuccessResponse())
	}
}

func GetUserJokesList(u app.JokesUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		jokes, err := u.GetUserJokesList(c.Context(), c.Locals("username").(string))
		if err != nil {
			switch err {
			default:
				c.Status(http.StatusInternalServerError)
			}
			return c.JSON(errorResponse(err))
		}

		return c.JSON(jokesListSuccessResponse(jokes))
	}
}

func GetThemeJokes(u app.JokesUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		jokes, err := u.GetThemeJokes(c.Context(), c.Params("theme"))
		if err != nil {
			switch err {
			default:
				c.Status(http.StatusInternalServerError)
			}
			return c.JSON(errorResponse(err))
		}

		return c.JSON(jokesListSuccessResponse(jokes))
	}
}
