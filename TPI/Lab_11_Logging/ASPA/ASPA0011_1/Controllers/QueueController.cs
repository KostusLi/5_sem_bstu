using Microsoft.AspNetCore.Mvc;
using ASPA0011_1.Models;
using ASPA0011_1.Services;

namespace ASPA0011_1.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class QueueController : ControllerBase
    {
        private readonly ChannelManager _manager;

        public QueueController(ChannelManager manager)
        {
            _manager = manager;
        }

        [HttpPost]
        public async Task<IActionResult> ExecuteQueueOperation([FromBody] QueueRequest req)
        {
            var (data, error) = await _manager.QueueOperationAsync(req.Id, req.Command, req.Data);

            if (error != null)
                return NotFound(new ErrorResponse { Id = req.Id, Error = error });

            if (req.Command == CommandQueue.enqueue)
                return Ok(new QueueResponse { Id = req.Id, Data = "Сообщение поставлено в очередь" });

            return Ok(new QueueResponse { Id = req.Id, Data = data });
        }
    }
}