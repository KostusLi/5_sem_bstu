using Microsoft.AspNetCore.Mvc;
using ASPA0011_1.Models;
using ASPA0011_1.Services;

namespace ASPA0011_1.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class ChannelsController : ControllerBase
    {
        private readonly ChannelManager _manager;
        private readonly ILogger<ChannelsController> _logger;

        public ChannelsController(ChannelManager manager, ILogger<ChannelsController> logger)
        {
            _manager = manager;
            _logger = logger;
            _logger.LogTrace("Создание ChannelsController");
        }

        [HttpGet]
        public IActionResult GetChannels()
        {
            var channels = _manager.GetAllChannels();
            return channels.Count > 0 ? Ok(channels) : NoContent();
        }

        [HttpGet("{id:guid}")]
        public IActionResult GetChannel(Guid id)
        {
            var channel = _manager.GetChannel(id);
            if (channel == null)
            {
                _logger.LogError($"Канал {id} не найден (404)");
                return NotFound();
            }
            return Ok(channel);
        }

        [HttpPost]
        public IActionResult CreateChannel([FromBody] ChannelRequest req)
        {
            if (req.Command != CommandChannels.@new) return BadRequest();
            var ch = _manager.CreateChannel(req.Name, req.Description, req.State);
            return StatusCode(201, ch);
        }

        [HttpPut]
        public IActionResult ChangeState([FromBody] ChannelRequest req)
        {
            if (req.Command == CommandChannels.close)
                _manager.ChangeState(req.Id, State.CLOSED);
            else if (req.Command == CommandChannels.open)
                _manager.ChangeState(req.Id, State.ACTIVE);
            else return BadRequest();

            return Ok(req.Id.HasValue ? _manager.GetChannel(req.Id.Value) : _manager.GetAllChannels());
        }

        [HttpDelete]
        public IActionResult DeleteChannels([FromBody] ChannelRequest req)
        {
            if (req.Command != CommandChannels.del) return BadRequest();

            _manager.DeleteChannels(req.State);

            var remaining = _manager.GetAllChannels();
            return remaining.Count > 0 ? Ok(remaining) : NoContent();
        }
    }
}