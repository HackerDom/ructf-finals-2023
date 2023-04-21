package server

import (
	"errors"
	"github.com/gofiber/fiber/v2"
	"net/http"
	"net/url"
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
			switch {
			case errors.Is(err, common.ErrAlreadyExists):
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
			switch {
			case errors.Is(err, common.ErrNotExists):
				c.Status(http.StatusBadRequest)
			case errors.Is(err, common.ErrInvalidPassword):
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

		joke, err := u.GetJoke(c.Context(), c.Locals("username").(string), body.Username, body.Theme)
		if err != nil {
			switch {
			case errors.Is(err, common.ErrNotExists):
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

		err := u.CreateJoke(c.Context(), c.Locals("username").(string), body.Status, body.Theme, body.Text)
		if err != nil {
			switch {
			case errors.Is(err, common.ErrInvalidStatus):
				c.Status(http.StatusBadRequest)
			case errors.Is(err, common.ErrAlreadyExists):
				c.Status(http.StatusBadRequest)
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
		theme, err := url.QueryUnescape(c.Params("theme"))
		if err != nil {
			c.Status(http.StatusInternalServerError)
			return c.JSON(errorResponse(err))
		}

		jokes, err := u.GetThemeJokes(c.Context(), c.Locals("username").(string), theme)
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

func CreateFriendRequest(u app.FriendsUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		var body createFriendRequestRequest

		if err := c.BodyParser(&body); err != nil {
			c.Status(http.StatusBadRequest)
			return c.JSON(errorResponse(err))
		}

		err := u.CreateFriendRequest(c.Context(), c.Locals("username").(string), body.To)
		if err != nil {
			switch {
			default:
				c.Status(http.StatusInternalServerError)
			}
			return c.JSON(errorResponse(err))
		}

		return c.JSON(emptySuccessResponse())
	}
}

func AcceptFriendRequest(u app.FriendsUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		var body acceptFriendRequestRequest

		if err := c.BodyParser(&body); err != nil {
			c.Status(http.StatusBadRequest)
			return c.JSON(errorResponse(err))
		}

		err := u.AcceptFriendRequest(c.Context(), c.Locals("username").(string), body.From)
		if err != nil {
			switch {
			default:
				c.Status(http.StatusInternalServerError)
			}
			return c.JSON(errorResponse(err))
		}

		return c.JSON(emptySuccessResponse())
	}
}

func GetFriendsList(u app.FriendsUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		friends, err := u.GetFriendsList(c.Context(), c.Locals("username").(string))
		if err != nil {
			switch {
			default:
				c.Status(http.StatusInternalServerError)
			}
			return c.JSON(errorResponse(err))
		}

		return c.JSON(friendsSuccessResponse(friends))
	}
}

func GetRequestsList(u app.FriendsUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		requests, err := u.GetRequestsList(c.Context(), c.Locals("username").(string))
		if err != nil {
			switch {
			default:
				c.Status(http.StatusInternalServerError)
			}
			return c.JSON(errorResponse(err))
		}

		return c.JSON(friendsSuccessResponse(requests))
	}
}
