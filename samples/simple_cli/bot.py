import json
import discord
import os
import sys

TOKEN = os.environ['BOT_TOKEN']
VOICE_GUILD_ID = os.environ['BOT_GUILD_ID']
VOICE_CHANNEL_ID = os.environ['BOT_CHANNEL_ID']

client = discord.Client()

@client.event
async def on_socket_response(data):
    if not data:
        return
    if data['op'] != 0:
        return

    if data['t'] == 'VOICE_SERVER_UPDATE':
        guild_id = data['d']['guild_id']
        user_id = str(client.user.id)
        endpoint = data['d']['endpoint']
        session = client.get_guild(int(guild_id)).me.voice.session_id
        token = data['d']['token']

        print("got info, passing back", file=sys.stderr)

        print(guild_id)
        print(user_id)
        print(endpoint)
        print(session)
        print(token)

        sys.stdout.flush()

        print("passed back", file=sys.stderr)

@client.event
async def on_ready():
    print('ready', file=sys.stderr)
    await client.ws.send(json.dumps({
        'op': 4,
        'd': {
            'self_deaf': True,
            'guild_id': VOICE_GUILD_ID,
            'channel_id': VOICE_CHANNEL_ID
        }
    }))

client.run(TOKEN)
