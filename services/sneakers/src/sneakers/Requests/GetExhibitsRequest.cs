using sneakers.Models;

namespace sneakers.Requests;

public record GetExhibitsRequest(
    List<FilterConditionJsonModel> Conditions,
    List<SneakersId> IdsToSearch = null
);